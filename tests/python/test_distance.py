"""Tests for Distance, Point, Size value types."""

import pytest


def test_distance_mm():
    from glabels.glabels_ext import Distance

    d = Distance.mm(25.4)
    assert abs(d.to_in() - 1.0) < 0.001
    assert abs(d.to_mm() - 25.4) < 0.001


def test_distance_inches():
    from glabels.glabels_ext import Distance

    d = Distance.in_(1.0)
    assert abs(d.to_pt() - 72.0) < 0.001
    assert abs(d.to_mm() - 25.4) < 0.001


def test_distance_arithmetic():
    from glabels.glabels_ext import Distance

    d1 = Distance.mm(10)
    d2 = Distance.mm(20)

    result = d1 + d2
    assert abs(result.to_mm() - 30.0) < 0.001

    result = d2 - d1
    assert abs(result.to_mm() - 10.0) < 0.001

    result = d1 * 3.0
    assert abs(result.to_mm() - 30.0) < 0.001

    result = d2 / 2.0
    assert abs(result.to_mm() - 10.0) < 0.001


def test_distance_comparison():
    from glabels.glabels_ext import Distance

    d1 = Distance.mm(10)
    d2 = Distance.mm(20)

    assert d1 < d2
    assert d2 > d1
    assert d1 == Distance.mm(10)
    assert d1 != d2
    assert d1 <= d2
    assert d2 >= d1


def test_distance_repr():
    from glabels.glabels_ext import Distance

    d = Distance.mm(10)
    r = repr(d)
    assert "Distance" in r


def test_point():
    from glabels.glabels_ext import Distance, Point

    p = Point(Distance.mm(10), Distance.mm(20))
    assert abs(p.x().to_mm() - 10.0) < 0.001
    assert abs(p.y().to_mm() - 20.0) < 0.001


def test_size():
    from glabels.glabels_ext import Distance, Size

    s = Size(Distance.mm(100), Distance.mm(50))
    assert abs(s.w().to_mm() - 100.0) < 0.001
    assert abs(s.h().to_mm() - 50.0) < 0.001


def test_color_node():
    from glabels.glabels_ext import ColorNode

    cn = ColorNode((255, 0, 0, 255))
    assert not cn.is_field()


def test_color_node_field():
    from glabels.glabels_ext import ColorNode

    cn = ColorNode("my_color_field")
    assert cn.is_field()
    assert cn.key() == "my_color_field"


def test_text_node():
    from glabels.glabels_ext import TextNode

    tn = TextNode(False, "Hello World")
    assert not tn.is_field()
    assert tn.data() == "Hello World"


def test_text_node_field():
    from glabels.glabels_ext import TextNode

    tn = TextNode(True, "name")
    assert tn.is_field()
    assert tn.data() == "name"


def test_units():
    from glabels.glabels_ext import Units, UnitsEnum

    u = Units(UnitsEnum.MM)
    assert u.to_id_string() == "mm"
