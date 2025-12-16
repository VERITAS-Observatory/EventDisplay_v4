"""Training variables for XGBoost direction reconstruction."""


def xgb_per_telescope_training_variables():
    """
    Telescope-type training variables for XGB.

    Disp variables with different indexing logic in data preparation.
    """

    return [
        "Disp_T",
        "DispXoff_T",
        "DispYoff_T",
        "DispWoff_T",
        "E",
        "ES",
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


def xgb_array_training_variables():
    """Array-level training variables for XGB."""
    return [
        "DispNImages",
        "DispTelList_T",
        "Xoff",
        "Yoff",
        "Xoff_intersect",
        "Yoff_intersect",
        "Erec",
        "ErecS",
        "EmissionHeight",
    ]


def xgb_all_training_variables():
    """All training variables for XGB."""
    return xgb_per_telescope_training_variables() + xgb_array_training_variables()
