"""Tests for rendering labels to PDF."""

import os
import tempfile

import pytest


def test_render_empty_model():
    """Rendering an empty model should not crash."""
    from glabels.glabels_ext import (
        Db,
        Distance,
        Model,
        PageRenderer,
        render_to_pdf,
    )

    model = Model()
    # Set a template so rendering has page dimensions
    templates = Db.templates()
    if templates:
        model.set_tmplate(templates[0])

        renderer = PageRenderer(model)
        renderer.set_n_copies(1)

        with tempfile.NamedTemporaryFile(suffix=".pdf", delete=False) as f:
            tmpfile = f.name

        try:
            render_to_pdf(renderer, tmpfile)
            assert os.path.exists(tmpfile)
            assert os.path.getsize(tmpfile) > 0
        finally:
            os.unlink(tmpfile)


def test_render_model_to_pdf_convenience():
    """Test the render_model_to_pdf convenience function."""
    from glabels.glabels_ext import (
        ColorNode,
        Db,
        Distance,
        FontWeight,
        Model,
        ModelTextObject,
        WrapMode,
        render_model_to_pdf,
    )

    model = Model()
    templates = Db.templates()
    if not templates:
        pytest.skip("No templates available")

    model.set_tmplate(templates[0])

    obj = ModelTextObject()
    obj.set_x0(Distance.mm(5))
    obj.set_y0(Distance.mm(5))
    obj.set_w(Distance.mm(60))
    obj.set_h(Distance.mm(10))
    obj.set_text("Test Label")
    model.add_object(obj)

    with tempfile.NamedTemporaryFile(suffix=".pdf", delete=False) as f:
        tmpfile = f.name

    try:
        render_model_to_pdf(model, tmpfile, copies=2)
        assert os.path.exists(tmpfile)
        assert os.path.getsize(tmpfile) > 0
    finally:
        os.unlink(tmpfile)


def test_page_renderer_info():
    """Test PageRenderer item/page counting."""
    from glabels.glabels_ext import Db, Model, PageRenderer

    model = Model()
    templates = Db.templates()
    if not templates:
        pytest.skip("No templates available")

    model.set_tmplate(templates[0])

    renderer = PageRenderer(model)
    n_labels = model.frame().n_labels()
    renderer.set_n_copies(n_labels)
    renderer.set_start_item(0)

    assert renderer.n_items() == n_labels
    assert renderer.n_pages() >= 1
