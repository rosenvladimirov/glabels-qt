"""
glabels - Python bindings for gLabels-qt label designer.

Usage:
    import glabels

    # Open an existing label
    label = glabels.Label.open("my-labels.glabels")
    label.set_merge_source("csv", "contacts.csv")
    label.render_pdf("output.pdf")

    # Create a new label
    label = glabels.Label(template="Avery 5160")
    label.add_text(
        x=glabels.mm(5), y=glabels.mm(5),
        w=glabels.mm(60), h=glabels.mm(10),
        text="${name}", font_size=12,
    )
    label.render_pdf("labels.pdf")
"""

import os as _os


def _use_bundled_resources():
    """Point Qt and gLabels at the resources shipped inside the wheel.

    A self-contained wheel carries its own Qt plugins, template database and
    fonts next to this file. Without them the library aborts the whole process
    (qFatal: "Cannot locate system template directory!"), or renders labels
    without a single glyph when the system has no fonts. The environment must
    be set before the extension module initialises Qt.
    """
    here = _os.path.dirname(_os.path.abspath(__file__))
    templates = _os.path.join(here, "templates")
    plugins = _os.path.join(here, "plugins")
    fonts_conf = _os.path.join(here, "fonts.conf")
    if _os.path.isdir(templates):
        _os.environ.setdefault("GLABELS_TEMPLATES_DIR", templates)
    if _os.path.isdir(plugins):
        # the bundled Qt only works with plugins built against it
        _os.environ["QT_PLUGIN_PATH"] = plugins
    if _os.path.isfile(fonts_conf):
        _os.environ.setdefault("FONTCONFIG_FILE", fonts_conf)
    if not (_os.environ.get("DISPLAY") or _os.environ.get("WAYLAND_DISPLAY")):
        # headless server: Qt starts a GUI application even to render a PDF
        _os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")


_use_bundled_resources()

from glabels import glabels_ext as _ext

# Auto-initialize all subsystems on import
_ext.init_all()

# Re-export core C++ types
from glabels.glabels_ext import (
    Distance,
    Point,
    Size,
    Units,
    UnitsEnum,
    ColorNode,
    TextNode,
    Template,
    Frame,
    FrameRect,
    Layout,
    Model,
    ModelObject,
    ModelTextObject,
    ModelBarcodeObject,
    ModelBoxObject,
    ModelEllipseObject,
    ModelLineObject,
    ModelImageObject,
    Variable,
    VariableType,
    VariableIncrement,
    Variables,
    Merge,
    Record,
    MergeFactory,
    MergeSourceType,
    PageRenderer,
    BarcodeStyle,
    BarcodeBackends,
    Db,
    FontWeight,
    WrapMode,
    render_to_pdf,
    render_model_to_pdf,
)

# I/O functions (exposed as module-level functions, not classes)
from glabels.glabels_ext import read_file, write_file
from glabels.glabels_ext import io as _io

# Convenience aliases matching the old class-based API
class XmlLabelParser:
    read_file = staticmethod(_io.read_file)
    read_buffer = staticmethod(_io.read_buffer)

class XmlLabelCreator:
    write_file = staticmethod(_io.write_file)
    write_buffer = staticmethod(_io.write_buffer)

# Convenience distance constructors
def mm(value):
    """Create a Distance in millimeters."""
    return Distance.mm(value)

def cm(value):
    """Create a Distance in centimeters."""
    return Distance.cm(value)

def inches(value):
    """Create a Distance in inches."""
    return Distance.in_(value)

def pt(value):
    """Create a Distance in points."""
    return Distance.pt(value)

# High-level API
from glabels.label import Label
from glabels.template_db import (
    find_template,
    list_templates,
    list_papers,
    list_categories,
)

def version():
    """Get the gLabels version string."""
    return _ext.version()

__version__ = "0.1.0"
