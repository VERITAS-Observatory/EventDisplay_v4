"""
Train XGBoost Multi-Target BDTs for Direction Reconstruction
=================================================

Uses x,y offsets calculated from intersection and dispBDT methods plus
image parameters to train multi-target regression BDTs to predict x,y offsets.

Separate BDTs are trained for 2, 3, and 4 telescope multiplicity events.

"""

import argparse
import logging
import os
from typing import Dict, List

import numpy as np
import pandas as pd
import uproot
import xgboost as xgb
from joblib import dump
from sklearn.metrics import mean_absolute_error, mean_squared_error
from sklearn.model_selection import train_test_split
from sklearn.multioutput import MultiOutputRegressor

logging.basicConfig(level=logging.INFO)
_logger = logging.getLogger("trainXGBoostforDirection")

# Telescope-type training variables
# NOTE: Disp_T must be first due to special indexing logic in data prep
TRAINING_VARIABLES = [
    "Disp_T",
    "DispXoff_T",
    "DispYoff_T",
    "DispWoff_T",
    "cen_x",
    "cen_y",
    "cosphi",
    "sinphi",
    "loss",
    "size",
    "dist",
    "width",
    "length",
    "asym",
    "tgrad_x",
]
N_TEL_VAR = len(TRAINING_VARIABLES)


def load_and_flatten_data(input_files, n_tel, max_events, training_step=True):
    """
    Reads the data from ROOT files, filters for the required multiplicity (n_tel),
    and flattens the telescope-array features into a Pandas DataFrame.
    """
    _logger.info(f"\n--- Loading and Flattening Data for n_tel = {n_tel} ---")
    _logger.info(
        f"Max events to process: {max_events if max_events > 0 else 'All available'}"
    )

    branch_list = [
        "DispNImages",
        "DispTelList_T",
        "DispXoff_T",
        "DispYoff_T",
        "DispWoff_T",
        "Xoff",
        "Yoff",
        "Xoff_intersect",
        "Yoff_intersect",
        "MCxoff",
        "MCyoff",
        "MCe0",
    ] + [var for var in TRAINING_VARIABLES]

    dfs = []
    if max_events > 0:
        max_events_per_file = max_events // len(input_files)
    else:
        max_events_per_file = None

    for f in input_files:
        try:
            with uproot.open(f) as root_file:
                if "data" in root_file:
                    _logger.info(f"Processing file: {f}")
                    tree = root_file["data"]
                    # read only needed branches
                    df = tree.arrays(branch_list, library="pd")
                    # Filter for n_tel
                    df = df[df["DispNImages"] == n_tel]
                    _logger.info(f"Number of events after n_tel filter: {len(df)}")
                    # Limit to max_events_per_file
                    if max_events_per_file and len(df) > max_events_per_file:
                        df = df.sample(n=max_events_per_file, random_state=42)
                    if not df.empty:
                        dfs.append(df)
                else:
                    _logger.warning(f"File: {f} does not contain a 'data' tree.")
        except Exception as e:
            _logger.error(f"Error opening or reading file {f}: {e}")

    if len(dfs) == 0:
        _logger.error("No data loaded from input files.")
        return pd.DataFrame()

    data_tree = pd.concat(dfs, ignore_index=True)
    _logger.info(f"Total events for n_tel={n_tel}: {len(data_tree)}")

    if data_tree.empty:
        return pd.DataFrame()

    df = data_tree
    flat_features = {}

    # 2. Flatten array variables and apply complex indexing logic (Disp_T vs others)
    for _, var_name in enumerate(TRAINING_VARIABLES):
        for i in range(n_tel):
            col_name = f"{var_name}_{i}"
            if var_name.startswith("Disp"):
                # Disp variables use index i (its position in the disp list)
                flat_features[col_name] = df[var_name].apply(lambda x: x[i])
            else:
                tel_indices = df["DispTelList_T"].apply(lambda x: x[i])
                var_arrays = df[var_name]

                def get_tel_value_at_index(var_array, tel_idx):
                    try:
                        tel_idx = int(tel_idx)
                        if 0 <= tel_idx < len(var_array):
                            return var_array[tel_idx]
                        else:
                            _logger.warning(
                                f"Warning: Index out of range for {var_name}, "
                                f"idx={tel_idx}, array length={len(var_array)}"
                            )
                            return np.nan
                    except (IndexError, TypeError) as e:
                        _logger.warning(
                            f"Warning: Error accessing {var_name}, "
                            f"arr={var_array}, idx={tel_idx}, error={e}"
                        )
                        return np.nan

                flat_features[col_name] = [
                    get_tel_value_at_index(arr, idx)
                    for arr, idx in zip(var_arrays, tel_indices)
                ]

    df_flat = pd.DataFrame(flat_features, index=df.index)

    for i in range(n_tel):
        df_flat[f"disp_x_{i}"] = df_flat[f"Disp_T_{i}"] * df_flat[f"cosphi_{i}"]
        df_flat[f"disp_y_{i}"] = df_flat[f"Disp_T_{i}"] * df_flat[f"sinphi_{i}"]

    df_flat["Xoff_weighted_bdt"] = df["Xoff"]
    df_flat["Yoff_weighted_bdt"] = df["Yoff"]
    df_flat["Xoff_intersect"] = df["Xoff_intersect"]
    df_flat["Yoff_intersect"] = df["Yoff_intersect"]

    if training_step:
        df_flat["MCxoff"] = df["MCxoff"]
        df_flat["MCyoff"] = df["MCyoff"]
        df_flat["MCe0"] = df["MCe0"]

    df_flat.dropna(inplace=True)
    _logger.info(f"Final events for n_tel={n_tel} after cleanup: {len(df_flat)}")

    return df_flat


def train_xgb_model(df, n_tel, TMVAOptions, output_dir, train_test_fraction):
    """
    Trains a single XGBoost model for multi-target regression (Xoff, Yoff).

    Parameters:
    - df: Pandas DataFrame with training data.
    - n_tel: Telescope multiplicity.
    - TMVAOptions: TMVA options string to configure XGBoost parameters.
    - output_dir: Directory to save the trained model.
    - train_test_fraction: Fraction of data to use for training.
    """
    if df.empty:
        print(f"Skipping training for n_tel={n_tel} due to empty data.")
        return

    # Features (X) and targets (Y)
    X_cols = [col for col in df.columns if col not in ["MCxoff", "MCyoff", "MCe0"]]
    X = df[X_cols]
    Y = df[["MCxoff", "MCyoff"]]

    X_train, X_test, Y_train, Y_test = train_test_split(
        X, Y, test_size=1.0 - train_test_fraction, random_state=42
    )

    _logger.info(
        f"n_tel={n_tel}: Training events: {len(X_train)}, Testing events: {len(X_test)}"
    )

    # Parse TMVA options (simplified mapping to XGBoost parameters)
    # The default TMVA string is:
    # !V:NTrees=800:BoostType=Grad:Shrinkage=0.1:MaxDepth=4:MinNodeSize=1.0%

    # Note: XGBoost defaults to gbtree (Gradient Boosting).
    # MultiOutputRegressor requires a base estimator (e.g., plain XGBRegressor)
    xgb_params = {
        "n_estimators": 800,
        "learning_rate": 0.1,  # Shrinkage
        "max_depth": 8,
        "min_child_weight": 1.0,  # Equivalent to MinNodeSize=1.0% for XGBoost
        "objective": "reg:squarederror",
        "n_jobs": -1,
        "random_state": 42,
        "tree_method": "hist",
        "subsample": 0.7,  # Default sensible value
        "colsample_bytree": 0.7,  # Default sensible value
    }

    if TMVAOptions:
        for opt in TMVAOptions.split(":"):
            if "=" in opt:
                key, val = opt.split("=")
                if key == "NTrees":
                    xgb_params["n_estimators"] = int(val)
                elif key == "Shrinkage":
                    xgb_params["learning_rate"] = float(val)
                elif key == "MaxDepth":
                    xgb_params["max_depth"] = int(val)

    base_estimator = xgb.XGBRegressor(**xgb_params)
    model = MultiOutputRegressor(base_estimator)
    _logger.info(f"Starting Multi-Target XGBoost Training for n_tel={n_tel}...")
    model.fit(X_train, Y_train)

    output_filename = os.path.join(output_dir, f"dispdir_bdt_ntel{n_tel}.joblib")
    dump(model, output_filename)
    _logger.info(f"XGBoost model saved to: {output_filename}")

    evaluate_model(model, X_test, Y_test, df, X_cols, Y)


def evaluate_model(model, X_test, Y_test, df, X_cols, Y):
    """
    Evaluates the trained model on the test set and prints performance metrics.

    """
    score = model.score(X_test, Y_test)
    _logger.info(f"XGBoost Multi-Target R^2 Score (Testing Set): {score:.4f}")
    Y_pred = model.predict(X_test)
    mse_x = mean_squared_error(Y_test["MCxoff"], Y_pred[:, 0])
    mse_y = mean_squared_error(Y_test["MCyoff"], Y_pred[:, 1])
    _logger.info(f"MSE (X_off): {mse_x:.4f}, MSE (Y_off): {mse_y:.4f}")
    mae_x = mean_absolute_error(Y_test["MCxoff"], Y_pred[:, 0])
    mae_y = mean_absolute_error(Y_test["MCyoff"], Y_pred[:, 1])
    _logger.info(f"MAE (X_off): {mae_x:.4f}, MAE (Y_off): {mae_y:.4f}")

    feature_importance(model, X_cols, Y.columns)

    angular_resolution(
        Y_pred,
        Y_test,
        df,
        percentiles=[68, 80, 90],
        log_e_min=-1,
        log_e_max=2,
        n_bins=5,
    )


def angular_resolution(Y_pred, Y_test, df, percentiles, log_e_min, log_e_max, n_bins):
    """
    Computes and logs the angular resolution based on predicted and true offsets.
    """
    results_df = pd.DataFrame(
        {
            "MCxoff_true": Y_test["MCxoff"].values,
            "MCyoff_true": Y_test["MCyoff"].values,
            "MCxoff_pred": Y_pred[:, 0],
            "MCyoff_pred": Y_pred[:, 1],
            "MCe0": df.loc[Y_test.index, "MCe0"].values,
        }
    )

    results_df["DeltaTheta"] = np.sqrt(
        (results_df["MCxoff_true"] - results_df["MCxoff_pred"]) ** 2
        + (results_df["MCyoff_true"] - results_df["MCyoff_pred"]) ** 2
    )

    results_df["LogE"] = np.log10(results_df["MCe0"])
    bins = np.linspace(log_e_min, log_e_max, n_bins + 1)
    results_df["E_bin"] = pd.cut(results_df["LogE"], bins=bins, include_lowest=True)
    results_df.dropna(subset=["E_bin"], inplace=True)

    resolution_results: Dict[str, List[float]] = {}
    mean_logE_by_bin = (
        results_df.groupby("E_bin", observed=False)["LogE"].mean().round(3)
    )

    # Group and calculate the specified percentile for DeltaTheta
    for p in percentiles:
        p_res = results_df.groupby("E_bin", observed=False)["DeltaTheta"].agg(
            lambda x: np.percentile(x, p) if len(x) > 0 else np.nan
        )
        resolution_results[f"Theta_{p}%"] = p_res.values

    output_df = pd.DataFrame(resolution_results, index=mean_logE_by_bin.index)
    output_df.insert(0, "Mean Log10(E)", mean_logE_by_bin.values)
    output_df.index.name = "Log10(E) Bin Range"
    output_df = output_df.dropna()  # Drop bins with no events

    _logger.info("\n--- Angular Resolution vs. Log10(MCe0) ---")
    _logger.info(
        f"Calculated over {n_bins} bins between Log10(E) = {log_e_min} and {log_e_max}"
    )
    _logger.info("\n" + output_df.to_markdown(floatfmt=".4f"))


def feature_importance(model, X_cols, target_names):
    """
    Prints feature importance from the trained XGBoost model.
    """
    _logger.info("--- XGBoost Multi-Regression Feature Importance ---")
    for i, estimator in enumerate(model.estimators_):
        target = target_names[i]
        _logger.info(f"\n### Importance for Target: **{target}**")

        importances = estimator.feature_importances_
        importance_df = pd.DataFrame({"Feature": X_cols, "Importance": importances})

        importance_df = importance_df.sort_values(by="Importance", ascending=False)
        _logger.info("\n" + importance_df.head(15).to_markdown(index=False))


def main():
    parser = argparse.ArgumentParser(
        description=("Train XGBoost Multi-Target BDTs for Direction Reconstruction")
    )
    parser.add_argument("input_file_list", help="List of input mscw ROOT files.")
    parser.add_argument(
        "output_dir", help="Output directory for XGBoost models and weights."
    )
    parser.add_argument(
        "train_test_fraction",
        type=float,
        help="Fraction of data for training (e.g., 0.5).",
    )
    parser.add_argument(
        "max_events",
        type=int,
        help="Maximum number of events to process across all files.",
    )
    parser.add_argument(
        "--tmva_options",
        default="!V:NTrees=800:BoostType=Grad:Shrinkage=0.1:MaxDepth=8:MinNodeSize=1.0%",
        help="TMVA options string (used to configure XGBoost parameters).",
    )

    args = parser.parse_args()

    try:
        with open(args.input_file_list, "r") as f:
            input_files = [line.strip() for line in f if line.strip()]
    except FileNotFoundError as exc:
        raise FileNotFoundError(
            f"Error: Input file list not found: {args.input_file_list}"
        ) from exc

    if not os.path.exists(args.output_dir):
        os.makedirs(args.output_dir)

    _logger.info("--- XGBoost Multi-Target Direction Training ---")
    _logger.info(f"Input files: {len(input_files)}")
    _logger.info(f"Output directory: {args.output_dir}")
    _logger.info(
        f"Train vs test fraction: {args.train_test_fraction}, Max events: {args.max_events}"
    )
    _logger.info(f"TMVA/XGB options: {args.tmva_options}")

    # Train separate BDTs for 2, 3, and 4 telescope multiplicity
    for n_tel in range(2, 5):
        df_flat = load_and_flatten_data(input_files, n_tel, args.max_events)
        train_xgb_model(
            df_flat, n_tel, args.tmva_options, args.output_dir, args.train_test_fraction
        )
    _logger.info("\nAll XGBoost models trained successfully.")


if __name__ == "__main__":
    main()
