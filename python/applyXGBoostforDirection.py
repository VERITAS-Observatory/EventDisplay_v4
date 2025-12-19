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


def get_branch_list():
    """Branches required for inference."""
    return [
        "DispNImages",
        "DispTelList_T",
        "Xoff",
        "Yoff",
        "Xoff_intersect",
        "Yoff_intersect",
        "fpointing_dx",
        "fpointing_dy",
    ] + [var for var in xgb_training_variables()]


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


def apply_image_selection(df, selected_indices):
    """Filter and pad telescope lists for selected indices."""
    if selected_indices is None or len(selected_indices) == 4:
        return df

    selected_set = set(selected_indices)

    def calculate_intersection(tel_list):
        return [tel_idx for tel_idx in tel_list if tel_idx in selected_set]

    df = df.copy()
    df["DispTelList_T_new"] = df["DispTelList_T"].apply(calculate_intersection)
    df["DispNImages_new"] = df["DispTelList_T_new"].apply(len)

    _logger.info(
        f"\n{df[['DispNImages', 'DispTelList_T', 'DispNImages_new', 'DispTelList_T_new']].head(20).to_string()}"
    )

    df["DispTelList_T"] = df["DispTelList_T_new"]
    df["DispNImages"] = df["DispNImages_new"]
    df = df.drop(columns=["DispTelList_T_new", "DispNImages_new"])

    for var_name in xgb_training_variables():
        if var_name in df.columns:
            df[var_name] = df[var_name].apply(
                lambda x: np.pad(
                    np.array(x),
                    (0, 4 - len(x)),
                    mode="constant",
                    constant_values=np.nan,
                )
                if isinstance(x, (list, np.ndarray))
                else x
            )

    return df


def filter_by_telescope_selection(df, selected_indices):
    """Return a boolean mask: True when all selected telescopes are present or event has 4 images."""
    if not selected_indices:
        return pd.Series([True] * len(df), index=df.index)

    selected_set = set(selected_indices)

    def ok(tel_list):
        if len(tel_list) >= 4:
            return True
        return selected_set.issubset(set(tel_list))

    return df["DispTelList_T"].apply(ok)


def flatten_data_vectorized(df, n_tel, training_variables):
    """
    Vectorized flattening of telescope array columns.
    Significantly faster than row-by-row iteration.
    """
    flat_features = {}
    tel_list_col = "DispTelList_T"

    def to_padded_array(arrays):
        """Convert list of variable-length arrays to fixed-size numpy array, padding with NaN."""
        max_len = max(len(arr) if hasattr(arr, "__len__") else 1 for arr in arrays)
        result = np.full((len(arrays), max_len), np.nan)
        for i, arr in enumerate(arrays):
            if hasattr(arr, "__len__"):
                result[i, : len(arr)] = arr
            else:
                result[i, 0] = arr
        return result

    def to_dense_array(col):
        """
        Convert a column of variable-length telescope data to a dense 2D numpy array.

        Handles uproot's awkward-style variable-length arrays from ROOT files
        by converting to plain Python lists first to avoid per-element iteration overhead.
        """
        # Convert to a plain Python list first to avoid per-element iteration overhead
        # when dealing with uproot's awkward-style variable-length arrays
        arrays = col.tolist() if hasattr(col, "tolist") else list(col)
        try:
            return np.vstack(arrays)
        except (ValueError, TypeError):
            return to_padded_array(arrays)

    tel_list_matrix = to_dense_array(df[tel_list_col])

    for var_name in training_variables:
        # Convert the column of arrays to a 2D numpy matrix
        # Shape: (n_events, max_n_tel)
        data_matrix = to_dense_array(df[var_name])

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


def load_models(model_dir):
    """
    Load XGBoost models for different telescope multiplicities from a directory.

    Parameters
    ----------
    model_dir : str
        Path to the directory containing the trained model files
        named ``dispdir_bdt_ntel{n_tel}_xgboost.joblib``.

    Returns
    -------
    dict[int, Any]
        A dictionary mapping the number of telescopes (n_tel) to the
        corresponding loaded model objects. Only models whose files
        exist in ``model_dir`` are included.
    """
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
    return models


def apply_models(df, models_or_dir, selection_mask=None):
    """
    Apply trained XGBoost models to a DataFrame chunk.

    Parameters
    ----------
    df : pandas.DataFrame
        Chunk of events to process. Must contain at least the columns used in
        ``xgb_training_variables()`` plus the pointing information
        ("fpointing_dx", "fpointing_dy") and "DispNImages".
    models_or_dir : dict[int, Any] or str
        Either a preloaded models dictionary (as returned by :func:`load_models`)
        or a path to a model directory. If a string is provided, models are
        loaded on the fly to satisfy test expectations.
    selection_mask : pandas.Series or None
        Optional mask; False entries are marked with -999 in outputs.

    Returns
    -------
    pred_xoff : numpy.ndarray
        Array of predicted Xoff values for each event in the chunk, aligned
        with the index of ``df``.
    pred_yoff : numpy.ndarray
        Array of predicted Yoff values for each event in the chunk, aligned
        with the index of ``df``.
    """
    n_events = len(df)
    pred_xoff = np.full(n_events, np.nan, dtype=np.float32)
    pred_yoff = np.full(n_events, np.nan, dtype=np.float32)

    if isinstance(models_or_dir, str):
        models = load_models(models_or_dir)
    else:
        models = models_or_dir

    grouped = df.groupby("DispNImages")

    for n_tel, group_df in grouped:
        n_tel = int(n_tel)
        if int(n_tel) < 2:
            continue
        if n_tel not in models:
            _logger.warning(
                f"No model available for n_tel={n_tel}, skipping {len(group_df)} events"
            )
            continue

        _logger.info(f"Processing {len(group_df)} events with n_tel={n_tel}")

        training_vars_with_pointing = xgb_training_variables() + [
            "fpointing_dx",
            "fpointing_dy",
        ]
        df_flat = flatten_data_vectorized(group_df, n_tel, training_vars_with_pointing)

        excluded_columns = ["MCxoff", "MCyoff", "MCe0"]
        for n in range(n_tel):
            excluded_columns.append(f"fpointing_dx_{n}")
            excluded_columns.append(f"fpointing_dy_{n}")

        feature_cols = [col for col in df_flat.columns if col not in excluded_columns]
        X = df_flat[feature_cols]

        model = models[n_tel]
        predictions = model.predict(X)

        for i, idx in enumerate(group_df.index):
            pred_xoff[idx] = predictions[i, 0]
            pred_yoff[idx] = predictions[i, 1]

    if selection_mask is not None:
        pred_xoff = np.where(selection_mask, pred_xoff, -999.0)
        pred_yoff = np.where(selection_mask, pred_yoff, -999.0)

    return pred_xoff, pred_yoff


def process_file_chunked(
    input_file,
    model_dir,
    output_file,
    image_selection,
    max_events=None,
    chunk_size=500000,
):
    """
    Stream events from an input ROOT file in chunks, apply XGBoost models,
    and write direction predictions to an output ROOT file.

    Parameters
    ----------
    input_file : str
        Path to the input ROOT file containing a "data" TTree.
    model_dir : str
        Directory containing the trained XGBoost model files named
        ``dispdir_bdt_ntel{n_tel}_xgboost.joblib`` for different telescope
        multiplicities.
    output_file : str
        Path to the output ROOT file to create. The file will contain a
        "StereoAnalysis" TTree with ``Dir_Xoff`` and ``Dir_Yoff`` branches.
    image_selection : str
        String specifying which telescope indices to select, passed to
        :func:`parse_image_selection` to obtain the corresponding indices
        used by :func:`apply_image_selection`.
    max_events : int, optional
        Maximum number of events to process. If None (default), all
        available events in the input file are processed.
    chunk_size : int, optional
        Number of events to read and process per chunk. Larger values reduce
        I/O overhead but increase memory usage. Default is 500000.

    Returns
    -------
    None
        This function writes results directly to ``output_file`` and does not
        return a value.
    """
    models = load_models(model_dir)
    branch_list = get_branch_list()
    selected_indices = parse_image_selection(image_selection)

    _logger.info(f"Chunk size: {chunk_size}")
    if max_events:
        _logger.info(f"Maximum events to process: {max_events}")

    with uproot.recreate(output_file) as root_file:
        tree = root_file.mktree(
            "StereoAnalysis",
            {"Dir_Xoff": np.float32, "Dir_Yoff": np.float32},
        )

        total_processed = 0

        for df_chunk in uproot.iterate(
            f"{input_file}:data",
            branch_list,
            library="pd",
            step_size=chunk_size,
        ):
            if df_chunk.empty:
                continue

            df_chunk = apply_image_selection(df_chunk, selected_indices)
            if df_chunk.empty:
                continue

            # Stop early if we've reached max_events limit
            if max_events is not None and total_processed >= max_events:
                break

            # Reset index to local chunk indices (0, 1, 2, ...) to avoid
            # index out-of-bounds when indexing chunk-sized output arrays
            df_chunk = df_chunk.reset_index(drop=True)

            pred_xoff, pred_yoff = apply_models(df_chunk, models)

            tree.extend(
                {
                    "Dir_Xoff": np.asarray(pred_xoff, dtype=np.float32),
                    "Dir_Yoff": np.asarray(pred_yoff, dtype=np.float32),
                }
            )

            total_processed += len(df_chunk)
            _logger.info(f"Processed {total_processed} events so far")

    _logger.info(
        f"Streaming complete. Total processed events written: {total_processed}"
    )


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
            "Keeps events with all selected telescopes or 4-telescope events. "
            "Default is 15 (0b1111), which selects all 4 telescopes."
        ),
    )
    parser.add_argument(
        "--max-events",
        type=int,
        default=None,
        help="Maximum number of events to process (default: all events)",
    )
    parser.add_argument(
        "--chunk-size",
        type=int,
        default=500000,
        help="Number of events to process per chunk (default: 500000)",
    )
    args = parser.parse_args()

    start_time = time.time()
    _logger.info("--- XGBoost Multi-Target Direction Evaluation ---")
    _logger.info(f"Input file: {args.input_file}")
    _logger.info(f"Model directory: {args.model_dir}")
    _logger.info(f"Output file: {args.output_file}")
    if args.image_selection:
        _logger.info(f"Image selection: {args.image_selection}")

    process_file_chunked(
        input_file=args.input_file,
        model_dir=args.model_dir,
        output_file=args.output_file,
        image_selection=args.image_selection,
        max_events=args.max_events,
        chunk_size=args.chunk_size,
    )

    elapsed_time = time.time() - start_time
    _logger.info(f"Processing complete. Total time: {elapsed_time:.2f} seconds")


if __name__ == "__main__":
    main()
