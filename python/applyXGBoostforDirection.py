"""
Evaluate XGBoost BDTs for Direction reconstruction

This script applies trained XGBoost models to predict Xoff and Yoff
for each event in an input ROOT file. The output ROOT file contains
one row per input event, maintaining the original event order.
"""

import argparse
import logging
import os
import time

import joblib
import numpy as np
import pandas as pd
import uproot
from training_variables import xgb_training_variables

logging.basicConfig(level=logging.INFO)
_logger = logging.getLogger("applyXGBoostforDirection")


def parse_image_selection(image_selection_str):
    """
    Parse the image_selection parameter.

    Can be either:
    - Bit-coded value (e.g., 14 = 0b1110 = telescopes 1,2,3)
    - Comma-separated indices (e.g., "1,2,3")

    Returns a list of telescope indices.
    """
    if not image_selection_str:
        return None

    # Parse as comma-separated indices
    if "," in image_selection_str:
        try:
            indices = [int(x.strip()) for x in image_selection_str.split(",")]
            _logger.info(f"Image selection indices: {indices}")
            return indices
        except ValueError:
            pass

    # Parse as bit-coded value
    try:
        bit_value = int(image_selection_str)
        # Extract bit positions (0-indexed)
        indices = [i for i in range(4) if (bit_value >> i) & 1]
        _logger.info(f"Image selection from bit-coded value {bit_value}: {indices}")
        return indices
    except ValueError:
        raise ValueError(
            f"Invalid image_selection format: {image_selection_str}. "
            "Use bit-coded value (e.g., 14) or comma-separated indices (e.g., '1,2,3')"
        )


def filter_by_telescope_selection(df, selected_indices):
    """
    Build a selection mask for events where all selected telescope indices
    are present in DispTelList_T, OR events with 4 telescopes.

    IMPORTANT: Does not drop events. Returns a boolean mask of length len(df).
    Use this mask to process only selected events while preserving original order.

    Parameters
    ----------
    - df: DataFrame with DispTelList_T column
    - selected_indices: List of selected telescope indices (or None)

    Returns
    -------
    pd.Series[bool]: selection mask (True = selected, False = unselected)
    """
    if selected_indices is None:
        return pd.Series([True] * len(df), index=df.index)

    _logger.info(f"Filtering events for telescope selection: {selected_indices}")

    def has_selected_telescopes(tel_list):
        """Check if event has all selected telescopes or has 4 telescopes."""
        tel_set = set(tel_list)
        has_all_selected = all(idx in tel_set for idx in selected_indices)
        has_four_telescopes = len(tel_list) == 4
        return has_all_selected or has_four_telescopes

    mask = df["DispTelList_T"].apply(has_selected_telescopes)

    _logger.info(
        f"Selection: {mask.sum()} of {len(df)} events "
        f"({100 * mask.sum() / len(df):.1f}%)"
    )
    return mask


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
    ] + [var for var in xgb_training_variables()]

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


def flatten_data_vectorized(df, n_tel, training_variables):
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

    # Convert dictionary to DataFrame once at the end
    df_flat = pd.DataFrame(flat_features, index=df.index)

    # Ensure all columns are float type (uproot may load as awkward arrays)
    for col in df_flat.columns:
        df_flat[col] = df_flat[col].astype(np.float32)

    for i in range(n_tel):
        df_flat[f"disp_x_{i}"] = df_flat[f"Disp_T_{i}"] * df_flat[f"cosphi_{i}"]
        df_flat[f"disp_y_{i}"] = df_flat[f"Disp_T_{i}"] * df_flat[f"sinphi_{i}"]
        df_flat[f"loss_loss_{i}"] = df_flat[f"loss_{i}"] ** 2
        df_flat[f"loss_dist{i}"] = df_flat[f"loss_{i}"] * df_flat[f"dist_{i}"]
        df_flat[f"width_length_{i}"] = df_flat[f"width_{i}"] / (
            df_flat[f"length_{i}"] + 1e-6
        )
        df_flat[f"size_{i}"] = np.log10(df_flat[f"size_{i}"] + 1e-6)  # avoid log(0)
        # pointing corrections
        df_flat[f"cen_x_{i}"] = df_flat[f"cen_x_{i}"] + df_flat[f"fpointing_dx_{i}"]
        df_flat[f"cen_y_{i}"] = df_flat[f"cen_y_{i}"] + df_flat[f"fpointing_dy_{i}"]

    df_flat["Xoff_weighted_bdt"] = df["Xoff"].astype(np.float32)
    df_flat["Yoff_weighted_bdt"] = df["Yoff"].astype(np.float32)
    df_flat["Xoff_intersect"] = df["Xoff_intersect"].astype(np.float32)
    df_flat["Yoff_intersect"] = df["Yoff_intersect"].astype(np.float32)
    df_flat["Diff_Xoff"] = (df["Xoff"] - df["Xoff_intersect"]).astype(np.float32)
    df_flat["Diff_Yoff"] = (df["Yoff"] - df["Yoff_intersect"]).astype(np.float32)

    return df_flat


def apply_models(df, model_dir, selection_mask=None):
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
        model_filename = os.path.join(
            model_dir, f"dispdir_bdt_ntel{n_tel}_xgboost.joblib"
        )
        if os.path.exists(model_filename):
            _logger.info(f"Loading model: {model_filename}")
            models[n_tel] = joblib.load(model_filename)
        else:
            _logger.warning(f"Model not found: {model_filename}")

    # Initialize output arrays (NaN by default for events we can't predict)
    n_events = len(df)
    pred_xoff = np.full(n_events, np.nan)
    pred_yoff = np.full(n_events, np.nan)

    # Group selected events (if mask provided) by DispNImages for batch processing
    df_to_group = df.loc[selection_mask] if selection_mask is not None else df
    grouped = df_to_group.groupby("DispNImages")

    for n_tel, group_df in grouped:
        n_tel = int(n_tel)
        if n_tel not in models:
            _logger.warning(
                f"No model available for n_tel={n_tel}, skipping {len(group_df)} events"
            )
            continue

        _logger.info(f"Processing {len(group_df)} events with n_tel={n_tel}")

        # Add fpointing_dx and fpointing_dy to the training variables for flattening
        training_vars_with_pointing = xgb_training_variables() + [
            "fpointing_dx",
            "fpointing_dy",
        ]
        df_flat = flatten_data_vectorized(group_df, n_tel, training_vars_with_pointing)

        # Get feature columns (exclude MC truth if present)
        excluded_columns = ["MCxoff", "MCyoff", "MCe0"]
        for n in range(n_tel):
            excluded_columns.append(f"fpointing_dx_{n}")
            excluded_columns.append(f"fpointing_dy_{n}")
        _logger.info(f"Excluded columns: {excluded_columns}")
        feature_cols = [col for col in df_flat.columns if col not in excluded_columns]
        _logger.info(f"Feature columns: {feature_cols}")
        X = df_flat[feature_cols]

        model = models[n_tel]
        predictions = model.predict(X)

        for i, idx in enumerate(group_df.index):
            pred_xoff[idx] = predictions[i, 0]
            pred_yoff[idx] = predictions[i, 1]

    # Fill unselected events with -999 as requested
    if selection_mask is not None:
        unselected = ~selection_mask.values
        pred_xoff[unselected] = -999
        pred_yoff[unselected] = -999
        _logger.info(
            f"Filled {unselected.sum()} unselected events with -999 for predictions"
        )

    _logger.info("Predictions complete")
    _logger.info(
        f"Prediction arrays length: {len(pred_xoff)} (input events: {len(df)})"
    )
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
        root_file.mktree("StereoAnalysis", output_data)

    _logger.info(f"Output written successfully: {len(pred_xoff)} events")


def main():
    parser = argparse.ArgumentParser(
        description=("Apply XGBoost Multi-Target BDTs for Stereo Reconstruction")
    )
    parser.add_argument(
        "--input-file",
        required=True,
        metavar="INPUT.root",
        help="Path to input mscw ROOT file",
    )
    parser.add_argument(
        "--model-dir",
        required=True,
        metavar="MODEL_DIR",
        help="Directory containing XGBoost models",
    )
    parser.add_argument(
        "--output-file",
        required=True,
        metavar="OUTPUT.root",
        help="Output ROOT file path for predictions",
    )
    parser.add_argument(
        "--image-selection",
        type=str,
        default="15",
        help=(
            "Optional telescope selection. Can be bit-coded (e.g., 14 for telescopes 1,2,3) "
            "or comma-separated indices (e.g., '1,2,3'). "
            "Keeps events with all selected telescopes or 4-telescope events."
        ),
    )
    args = parser.parse_args()

    start_time = time.time()
    _logger.info("--- XGBoost Multi-Target Direction Evaluation ---")
    _logger.info(f"Input file: {args.input_file}")
    _logger.info(f"Model directory: {args.model_dir}")
    _logger.info(f"Output file: {args.output_file}")
    if args.image_selection:
        _logger.info(f"Image selection: {args.image_selection}")

    df = load_all_events(args.input_file, max_events=None)

    selection_mask = None
    if args.image_selection:
        selected_indices = parse_image_selection(args.image_selection)
        selection_mask = filter_by_telescope_selection(df, selected_indices)
    else:
        _logger.info("No image selection applied")

    pred_xoff, pred_yoff = apply_models(df, args.model_dir, selection_mask)
    write_output_root_file(args.output_file, pred_xoff, pred_yoff)

    elapsed_time = time.time() - start_time
    _logger.info(f"Processing complete. Total time: {elapsed_time:.2f} seconds")


if __name__ == "__main__":
    main()
