"""Tests for the high-level Label class."""

import pytest


def test_create_model():
    from glabels.glabels_ext import Model

    model = Model()
    assert model is not None
    assert model.is_selection_empty()
    assert len(model.object_list()) == 0


def test_create_text_object():
    from glabels.glabels_ext import (
        Distance,
        Model,
        ModelTextObject,
    )

    model = Model()
    obj = ModelTextObject()
    obj.set_x0(Distance.mm(5))
    obj.set_y0(Distance.mm(5))
    obj.set_w(Distance.mm(60))
    obj.set_h(Distance.mm(10))
    obj.set_text("Hello World")
    model.add_object(obj)
    assert len(model.object_list()) == 1
    # Delete reference to obj before model to avoid use-after-free
    del obj
    del model


def test_create_box_object():
    from glabels.glabels_ext import ColorNode, Distance, Model, ModelBoxObject

    model = Model()
    obj = ModelBoxObject(
        Distance.mm(0), Distance.mm(0),
        Distance.mm(50), Distance.mm(30),
        False,
        Distance.pt(1.0),
        ColorNode((0, 0, 0, 255)),
        ColorNode((255, 255, 255, 0)),
    )
    model.add_object(obj)
    assert len(model.object_list()) == 1
    assert abs(obj.w().to_mm() - 50.0) < 0.001
    del obj
    del model


def test_variables():
    from glabels.glabels_ext import Model, Variable, VariableType

    model = Model()
    var = Variable(VariableType.STRING, "company", "Acme Corp")
    model.variables().add_variable(var)
    assert model.variables().has_variable("company")
    assert model.variables()["company"].value() == "Acme Corp"


def test_variables_set_from_dict():
    from glabels.glabels_ext import Model

    model = Model()
    model.variables().set_variables({"name": "John", "city": "Sofia"})
    # set_variables creates STRING variables from dict
    assert model.variables().has_variable("name")
    assert model.variables().has_variable("city")


def test_merge_factory():
    from glabels.glabels_ext import MergeFactory

    names = MergeFactory.name_list()
    assert len(names) > 0
    # "None" should be in the list
    assert any("None" in n or "none" in n.lower() for n in names)


def test_barcode_backends():
    from glabels.glabels_ext import BarcodeBackends

    styles = BarcodeBackends.style_list()
    assert len(styles) > 0
    default = BarcodeBackends.default_style()
    assert default.name() != ""
