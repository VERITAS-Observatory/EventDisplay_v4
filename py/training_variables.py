"""Training variables for XGBoost direction reconstruction."""


def xgb_training_variables():
    """
    Telescope-type training variables for XGB.

    Disp variables with different indexing logic in data preparation.
    """

    return [
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
