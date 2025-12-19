import applyXGBoostforDirection as mod
import numpy as np
import pandas as pd


def test_parse_image_selection_indices():
    indices = mod.parse_image_selection("1,2,3")
    assert indices == [1, 2, 3]


def test_parse_image_selection_bits():
    indices = mod.parse_image_selection("14")  # 0b1110 -> [1,2,3]
    assert indices == [1, 2, 3]


def test_filter_by_telescope_selection_returns_mask_and_preserves_length():
    df = pd.DataFrame(
        {
            "DispTelList_T": [
                [1, 2, 3],  # has 1,2,3
                [1, 3],  # missing 2
                [0, 1, 2, 3],  # 4 telescopes -> always included
                [0, 2],  # missing 1,3
            ]
        }
    )
    selected = [1, 2, 3]
    mask = mod.filter_by_telescope_selection(df, selected)
    assert isinstance(mask, pd.Series)
    assert len(mask) == len(df)
    # Expect True for rows 0 and 2
    assert mask.tolist() == [True, False, True, False]


class DummyModel:
    def __init__(self, out_val=(0.0, 0.0)):
        self.out_val = np.array(out_val, dtype=float)

    def predict(self, X):
        # Return shape (n_rows, 2) filled with out_val
        n = len(X)
        return np.tile(self.out_val, (n, 1))


def test_apply_models_with_selection_mask(monkeypatch):
    # Build a minimal DataFrame with required columns
    df = pd.DataFrame(
        {
            "DispNImages": [3, 3, 4, 2, 3],
            "DispTelList_T": [
                [1, 2, 3],  # selected
                [1, 3],  # unselected
                [0, 1, 2, 3],  # 4 telescopes -> selected
                [0, 1],  # unselected
                [1, 2, 3],  # selected
            ],
            # Truth-related columns used downstream; values not relevant for this test
            "Xoff": [0, 0, 0, 0, 0],
            "Yoff": [0, 0, 0, 0, 0],
            "Xoff_intersect": [0, 0, 0, 0, 0],
            "Yoff_intersect": [0, 0, 0, 0, 0],
        }
    )

    # Selection: require telescopes 1,2,3 or len==4
    selection_mask = mod.filter_by_telescope_selection(df, [1, 2, 3])

    # Monkeypatch model loading and existence checks
    monkeypatch.setattr(mod.os.path, "exists", lambda p: True)

    # Always return a DummyModel; value differentiates by n_tel if desired
    def fake_load(path):
        if "ntel4" in path:
            return DummyModel(out_val=(4.0, -4.0))
        elif "ntel3" in path:
            return DummyModel(out_val=(3.0, -3.0))
        else:
            return DummyModel(out_val=(2.0, -2.0))

    monkeypatch.setattr(mod.joblib, "load", fake_load)

    # Monkeypatch flattening to avoid complex array inputs
    def fake_flatten(group_df, n_tel, training_vars):
        # Provide a simple feature column not in excluded list
        return pd.DataFrame({"feature": np.zeros(len(group_df))}, index=group_df.index)

    monkeypatch.setattr(mod, "flatten_data_vectorized", fake_flatten)

    pred_x, pred_y = mod.apply_models(df, "dummy_models_dir", selection_mask)

    # Output length must match input length
    assert len(pred_x) == len(df)
    assert len(pred_y) == len(df)

    # Unselected rows must be -999; selected rows come from model values
    # From selection_mask above: rows 0,2,4 are selected; 1,3 are unselected
    expected_x = [3.0, -999.0, 4.0, -999.0, 3.0]
    expected_y = [-3.0, -999.0, -4.0, -999.0, -3.0]
    assert np.allclose(pred_x, expected_x, equal_nan=False)
    assert np.allclose(pred_y, expected_y, equal_nan=False)
