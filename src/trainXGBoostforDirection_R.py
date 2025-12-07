"""
Train XGBoost Multi-Target BDTs for Direction Reconstruction
=================================================

Uses x,y offsets calculated from intersection and dispBDT methods plus
image parameters to train multi-target regression BDTs to predict R, phi offsets.

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

# Telescope-type training variables (read from ROOT file)
# Disp variables with different indexing logic in data preparation
TRAINING_VARIABLES_ROOT = [
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
    "R_core",
]

# Training variables after conversion to polar coordinates
TRAINING_VARIABLES = [
    "Disp_T",
    "Disp_R_T",
    "DispPhi_T",
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
    "R_core",
]
N_TEL_VAR = len(TRAINING_VARIABLES)


def load_and_flatten_data(
    input_files, n_tel, max_events, training_step=True, jitter_seed=None
):
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
        "Xoff",
        "Yoff",
        "Xoff_intersect",
        "Yoff_intersect",
        "MCxoff",
        "MCyoff",
        "MCe0",
    ] + [var for var in TRAINING_VARIABLES_ROOT]

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

    # Compute weights: R (to reflect physical sky area)
    mcr_raw = np.sqrt(data_tree["MCxoff"] ** 2 + data_tree["MCyoff"] ** 2)
    sample_weights = mcr_raw

    # jitter offsets to account for discrete training data in R
    rng = np.random.default_rng(jitter_seed)
    jitter_offsets = rng.uniform(-0.125, 0.125, len(data_tree))

    df_flat = flatten_data_vectorized(
        data_tree, n_tel, TRAINING_VARIABLES_ROOT, jitter_offsets=jitter_offsets
    )

    if training_step:
        df_flat["MCR"] = mcr_raw + jitter_offsets
        df_flat["MCphi"] = np.arctan2(data_tree["MCyoff"], data_tree["MCxoff"])
        df_flat["MCe0"] = data_tree["MCe0"]
        df_flat["sample_weight"] = sample_weights

    df_flat.dropna(inplace=True)
    _logger.info(f"Final events for n_tel={n_tel} after cleanup: {len(df_flat)}")

    return df_flat


def flatten_data_vectorized(df, n_tel, training_variables, jitter_offsets=None):
    """
    Vectorized flattening of telescope array columns.
    Significantly faster than row-by-row iteration.
    """
    flat_features = {}

    try:
        tel_list_matrix = np.vstack(df["DispTelList_T"].values)
    except ValueError:
        tel_list_matrix = np.array(df["DispTelList_T"].tolist())

    for var_name in training_variables:
        # Convert the column of arrays to a 2D numpy matrix
        # Shape: (n_events, max_n_tel)
        try:
            data_matrix = np.vstack(df[var_name].values)
        except ValueError:
            data_matrix = np.array(df[var_name].tolist())

        for i in range(n_tel):
            col_name = f"{var_name}_{i}"

            if var_name.startswith("Disp"):
                # Case 1: Simple index i
                if i < data_matrix.shape[1]:
                    flat_features[col_name] = data_matrix[:, i]
                else:
                    flat_features[col_name] = np.full(len(df), np.nan)
            else:
                # Case 2: Index lookup via DispTelList_T
                target_tel_indices = tel_list_matrix[:, i].astype(int)

                row_indices = np.arange(len(df))
                valid_mask = (target_tel_indices >= 0) & (
                    target_tel_indices < data_matrix.shape[1]
                )
                result = np.full(len(df), np.nan)
                result[valid_mask] = data_matrix[
                    row_indices[valid_mask], target_tel_indices[valid_mask]
                ]

                flat_features[col_name] = result

    df_flat = pd.DataFrame(flat_features, index=df.index)

    if jitter_offsets is None:
        jitter_offsets = 0.0

    for i in range(n_tel):
        disp_x = df_flat[f"DispXoff_T_{i}"]
        disp_y = df_flat[f"DispYoff_T_{i}"]
        df_flat[f"Disp_R_T_{i}"] = np.sqrt(disp_x**2 + disp_y**2)
        df_flat[f"DispPhi_T_{i}"] = np.arctan2(disp_y, disp_x)
        # Drop the Cartesian Disp columns as they're no longer needed
        df_flat.drop([f"DispXoff_T_{i}", f"DispYoff_T_{i}"], axis=1, inplace=True)

    for i in range(n_tel):
        df_flat[f"disp_x_{i}"] = df_flat[f"Disp_T_{i}"] * df_flat[f"cosphi_{i}"]
        df_flat[f"disp_y_{i}"] = df_flat[f"Disp_T_{i}"] * df_flat[f"sinphi_{i}"]

    df_flat["R_weighted_bdt"] = (
        np.sqrt(df["Xoff"] ** 2 + df["Yoff"] ** 2) + jitter_offsets
    )
    df_flat["phi_weighted_bdt"] = np.arctan2(df["Yoff"], df["Xoff"])
    df_flat["R_intersect"] = (
        np.sqrt(df["Xoff_intersect"] ** 2 + df["Yoff_intersect"] ** 2) + jitter_offsets
    )
    df_flat["phi_intersect"] = np.arctan2(df["Yoff_intersect"], df["Xoff_intersect"])

    return df_flat


def train_xgb_model(df, n_tel, output_dir, train_test_fraction):
    """
    Trains a single XGBoost model for multi-target regression (R, phi).

    Parameters:
    - df: Pandas DataFrame with training data.
    - n_tel: Telescope multiplicity.
    - output_dir: Directory to save the trained model.
    - train_test_fraction: Fraction of data to use for training.
    """
    if df.empty:
        _logger.warning(f"Skipping training for n_tel={n_tel} due to empty data.")
        return

    # Features (X) and targets (Y)
    X_cols = [
        col
        for col in df.columns
        if col not in ["MCR", "MCphi", "MCe0", "sample_weight"]
    ]
    X = df[X_cols]
    Y = df[["MCR", "MCphi"]]

    _logger.info(f"Training variables ({len(X_cols)}): {X_cols}")

    X_train, X_test, Y_train, Y_test, W_train, W_test = train_test_split(
        X,
        Y,
        df["sample_weight"],
        test_size=1.0 - train_test_fraction,
        random_state=42,
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
        "n_estimators": 1500,
        "learning_rate": 0.1,  # Shrinkage
        "max_depth": 4,
        "min_child_weight": 5.0,  # Equivalent to MinNodeSize=1.0% for XGBoost
        "objective": "reg:squarederror",
        "n_jobs": 4,
        "random_state": 42,
        "tree_method": "hist",
        "subsample": 0.7,  # Default sensible value
        "colsample_bytree": 0.7,  # Default sensible value
    }
    _logger.info(f"XGBoost parameters: {xgb_params}")

    _logger.info(
        f"Sample weights (MCR) - min: {W_train.min():.6f}, "
        f"max: {W_train.max():.6f}, mean: {W_train.mean():.6f}"
    )

    base_estimator = xgb.XGBRegressor(**xgb_params)
    model = MultiOutputRegressor(base_estimator)
    _logger.info(f"Starting Multi-Target XGBoost Training for n_tel={n_tel}...")
    model.fit(X_train, Y_train, sample_weight=W_train)

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
    mse_r = mean_squared_error(Y_test["MCR"], Y_pred[:, 0])
    mse_phi = mean_squared_error(Y_test["MCphi"], Y_pred[:, 1])
    _logger.info(f"MSE (R): {mse_r:.4f}, MSE (phi): {mse_phi:.4f}")
    mae_r = mean_absolute_error(Y_test["MCR"], Y_pred[:, 0])
    mae_phi = mean_absolute_error(Y_test["MCphi"], Y_pred[:, 1])
    _logger.info(f"MAE (R): {mae_r:.4f}, MAE (phi): {mae_phi:.4f}")

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
    Computes and logs the angular resolution based on predicted and true R, phi offsets.
    """
    # Convert R, phi predictions and targets to x, y for angular resolution
    mcr_true = Y_test["MCR"].values
    mcphi_true = Y_test["MCphi"].values
    mcr_pred = Y_pred[:, 0]
    mcphi_pred = Y_pred[:, 1]

    # Convert to Cartesian
    mcx_true = mcr_true * np.cos(mcphi_true)
    mcy_true = mcr_true * np.sin(mcphi_true)
    mcx_pred = mcr_pred * np.cos(mcphi_pred)
    mcy_pred = mcr_pred * np.sin(mcphi_pred)

    results_df = pd.DataFrame(
        {
            "MCx_true": mcx_true,
            "MCy_true": mcy_true,
            "MCx_pred": mcx_pred,
            "MCy_pred": mcy_pred,
            "MCe0": df.loc[Y_test.index, "MCe0"].values,
        }
    )

    results_df["DeltaTheta"] = np.sqrt(
        (results_df["MCx_true"] - results_df["MCx_pred"]) ** 2
        + (results_df["MCy_true"] - results_df["MCy_pred"]) ** 2
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
    parser.add_argument("ntel", type=int, help="Telescope multiplicity (2, 3, or 4).")
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
        "--jitter-seed",
        type=int,
        default=127,
        help="Random seed for per-event R jitter (default: based on 0.125).",
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
    _logger.info(f"Telescope multiplicity: {args.ntel}")
    _logger.info(f"Output directory: {args.output_dir}")
    _logger.info(
        f"Train vs test fraction: {args.train_test_fraction}, Max events: {args.max_events}"
    )

    df_flat = load_and_flatten_data(
        input_files,
        args.ntel,
        args.max_events,
        jitter_seed=args.jitter_seed,
    )
    train_xgb_model(df_flat, args.ntel, args.output_dir, args.train_test_fraction)
    _logger.info("\nXGBoost model trained successfully.")


if __name__ == "__main__":
    main()
