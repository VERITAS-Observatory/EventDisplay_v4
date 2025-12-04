"""
Evaluate XGBoost BDTs for Directory reconstruction

This script applies trained XGBoost models to predict Xoff and Yoff
for each event in an input ROOT file. The output ROOT file contains
one row per input event, maintaining the original event order.
"""

import argparse
import logging
import os

import joblib
import numpy as np
import pandas as pd
import uproot

from trainXGBoostforDirection import TRAINING_VARIABLES

logging.basicConfig(level=logging.INFO)
_logger = logging.getLogger("applyXGBoostforDirection")


def load_all_events(input_file, max_events=None):
    """
    Load all events from the input ROOT file without filtering by n_tel.
    Returns a DataFrame with all events in their original order.

    Parameters
    ----------

    - input_file: Path to the ROOT file
    - max_events: Maximum number of events to load (None = all events)
    """
    _logger.info(f"Loading events from {input_file}")
    if max_events:
        _logger.info(f"Maximum events to load: {max_events}")

    branch_list = [
        "DispNImages",
        "DispTelList_T",
        "Xoff",
        "Yoff",
        "Xoff_intersect",
        "Yoff_intersect",
        "fpointing_dx",
        "fpointing_dy",
    ] + [var for var in TRAINING_VARIABLES]

    with uproot.open(input_file) as root_file:
        if "data" not in root_file:
            raise ValueError(f"File {input_file} does not contain a 'data' tree.")

        tree = root_file["data"]
        if max_events:
            df = tree.arrays(branch_list, library="pd", entry_stop=max_events)
        else:
            df = tree.arrays(branch_list, library="pd")

    _logger.info(f"Loaded {len(df)} events")
    return df


def flatten_event_features(row, n_tel):
    """
    Flatten telescope-array features for a single event row.
    Returns a dictionary of flattened features.
    """
    flat_features = {}

    for var_name in TRAINING_VARIABLES:
        for i in range(n_tel):
            col_name = f"{var_name}_{i}"
            if var_name.startswith("Disp"):
                # Disp variables use index i (their position in the disp list)
                try:
                    flat_features[col_name] = row[var_name][i]
                except (IndexError, TypeError):
                    flat_features[col_name] = np.nan
            else:
                # Other variables use telescope index from DispTelList_T
                try:
                    tel_idx = int(row["DispTelList_T"][i])
                    if 0 <= tel_idx < len(row[var_name]):
                        flat_features[col_name] = row[var_name][tel_idx]
                    else:
                        flat_features[col_name] = np.nan
                except (IndexError, TypeError):
                    flat_features[col_name] = np.nan

    for i in range(n_tel):
        flat_features[f"disp_x_{i}"] = flat_features[f"Disp_T_{i}"] * flat_features[
            f"cosphi_{i}"
        ] + flat_features.get(f"fpointing_dx_{i}", 0)
        flat_features[f"disp_y_{i}"] = flat_features[f"Disp_T_{i}"] * flat_features[
            f"sinphi_{i}"
        ] + flat_features.get(f"fpointing_dy_{i}", 0)

    flat_features["Xoff_weighted_bdt"] = row["Xoff"]
    flat_features["Yoff_weighted_bdt"] = row["Yoff"]
    flat_features["Xoff_intersect"] = row["Xoff_intersect"]
    flat_features["Yoff_intersect"] = row["Yoff_intersect"]

    return flat_features


def apply_models(df, model_dir):
    """
    Apply trained XGBoost models to all events in the DataFrame.
    Returns arrays of predicted Xoff and Yoff for each event.

    Each event is processed with the model corresponding to its DispNImages value.
    Events are kept in their original order.
    """
    _logger.info("Loading models and applying predictions...")

    # Load all models (for n_tel = 2, 3, 4)
    models = {}
    for n_tel in range(2, 5):
        model_filename = os.path.join(model_dir, f"dispdir_bdt_ntel{n_tel}.joblib")
        if os.path.exists(model_filename):
            _logger.info(f"Loading model: {model_filename}")
            models[n_tel] = joblib.load(model_filename)
        else:
            _logger.warning(f"Model not found: {model_filename}")

    # Initialize output arrays (NaN by default for events we can't predict)
    n_events = len(df)
    pred_xoff = np.full(n_events, np.nan)
    pred_yoff = np.full(n_events, np.nan)

    # Group events by DispNImages for batch processing
    grouped = df.groupby("DispNImages")

    for n_tel, group_df in grouped:
        n_tel = int(n_tel)
        if n_tel not in models:
            _logger.warning(
                f"No model available for n_tel={n_tel}, skipping {len(group_df)} events"
            )
            continue

        _logger.info(f"Processing {len(group_df)} events with n_tel={n_tel}")

        flattened_rows = []
        indices = []
        for idx in group_df.index:
            row = group_df.loc[idx]
            flat_features = flatten_event_features(row, n_tel)
            flattened_rows.append(flat_features)
            indices.append(idx)

        df_flat = pd.DataFrame(flattened_rows, index=indices)

        # Get feature columns (exclude MC truth if present)
        feature_cols = [
            col
            for col in df_flat.columns
            if col not in ["MCxoff", "MCyoff", "MCe0"]
            and not col.startswith("fpointing_dx")
            and not col.startswith("fpointing_dy")
        ]
        X = df_flat[feature_cols]

        model = models[n_tel]
        predictions = model.predict(X)

        for i, idx in enumerate(group_df.index):
            pred_xoff[idx] = predictions[i, 0]
            pred_yoff[idx] = predictions[i, 1]

    _logger.info("Predictions complete")
    return pred_xoff, pred_yoff


def write_output_root_file(output_file, pred_xoff, pred_yoff):
    """
    Write predictions to a ROOT file with a single tree.
    Each row contains the predicted Xoff and Yoff for one event.
    """
    _logger.info(f"Writing output to {output_file}")

    output_data = {
        "Dir_Xoff": np.asarray(pred_xoff, dtype=np.float32),
        "Dir_Yoff": np.asarray(pred_yoff, dtype=np.float32),
    }

    with uproot.recreate(output_file) as root_file:
        root_file.mktree("DispDirection", output_data)

    _logger.info(f"Output written successfully: {len(pred_xoff)} events")


def main():
    parser = argparse.ArgumentParser(
        description=("Apply XGBoost Multi-Target BDTs for Direction Reconstruction")
    )
    parser.add_argument("input_file", help="Input mscw ROOT file.")
    parser.add_argument("model_dir", help="Directory with XGBoost models.")
    parser.add_argument("output_file", help="Output ROOT file with applied BDTs.")
    args = parser.parse_args()

    _logger.info("--- XGBoost Multi-Target Direction Evaluation ---")
    _logger.info(f"Input file: {args.input_file}")
    _logger.info(f"Model directory: {args.model_dir}")
    _logger.info(f"Output file: {args.output_file}")

    df = load_all_events(args.input_file, max_events=0)
    pred_xoff, pred_yoff = apply_models(df, args.model_dir)
    write_output_root_file(args.output_file, pred_xoff, pred_yoff)

    _logger.info("Processing complete.")


if __name__ == "__main__":
    main()
