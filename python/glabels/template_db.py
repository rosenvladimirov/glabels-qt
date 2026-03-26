"""Pythonic access to the gLabels template database."""

from glabels import glabels_ext as _ext


def find_template(name=None, brand=None, part=None):
    """Find a template by name or brand/part.

    Args:
        name: Full template name (e.g., "Avery 5160").
        brand: Template brand (e.g., "Avery").
        part: Template part number (e.g., "5160").

    Returns:
        A Template object, or None if not found.
    """
    if name is not None:
        tmpl = _ext.Db.lookup_template_from_name(name)
        if not tmpl.is_null():
            return tmpl
    if brand is not None and part is not None:
        tmpl = _ext.Db.lookup_template_from_brand_part(brand, part)
        if not tmpl.is_null():
            return tmpl
    return None


def list_templates(category=None, paper_size=None):
    """List available templates with optional filtering.

    Args:
        category: Filter by category id (e.g., "label").
        paper_size: Filter by paper id (e.g., "A4", "US-Letter").

    Returns:
        List of Template objects.
    """
    templates = _ext.Db.templates()
    if category is not None:
        templates = [t for t in templates if t.has_category(category)]
    if paper_size is not None:
        templates = [t for t in templates if t.paper_id() == paper_size]
    return templates


def list_papers():
    """List all known paper sizes.

    Returns:
        List of Paper objects.
    """
    return _ext.Db.papers()


def list_categories():
    """List all known template categories.

    Returns:
        List of Category objects.
    """
    return _ext.Db.categories()
