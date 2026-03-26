"""Tests for the template database."""

import pytest


def test_templates_not_empty():
    from glabels.glabels_ext import Db

    templates = Db.templates()
    assert len(templates) > 0


def test_papers_not_empty():
    from glabels.glabels_ext import Db

    papers = Db.papers()
    assert len(papers) > 0


def test_categories_not_empty():
    from glabels.glabels_ext import Db

    categories = Db.categories()
    assert len(categories) > 0


def test_lookup_paper():
    from glabels.glabels_ext import Db

    paper = Db.lookup_paper_from_id("A4")
    assert paper.id() == "A4"
    assert paper.width().to_mm() > 0


def test_template_properties():
    from glabels.glabels_ext import Db

    templates = Db.templates()
    t = templates[0]
    # Should have basic properties
    assert t.name() != ""
    assert not t.is_null()
    assert t.page_width().to_mm() > 0
    assert t.page_height().to_mm() > 0


def test_template_frame():
    from glabels.glabels_ext import Db

    templates = Db.templates()
    t = templates[0]
    frame = t.frame()
    assert frame is not None
    assert frame.n_labels() > 0
    assert frame.w().to_mm() > 0
    assert frame.h().to_mm() > 0


def test_pythonic_find_template():
    from glabels.template_db import find_template, list_templates

    # list_templates should work
    templates = list_templates()
    assert len(templates) > 0


def test_pythonic_list_papers():
    from glabels.template_db import list_papers

    papers = list_papers()
    assert len(papers) > 0


def test_pythonic_list_categories():
    from glabels.template_db import list_categories

    categories = list_categories()
    assert len(categories) > 0
