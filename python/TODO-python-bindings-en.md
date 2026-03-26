# TODO: Python Bindings for gLabels-qt

## Status: Working Prototype

All core features are implemented and tested.

## Tested and Working

- [x] Build infrastructure (pyproject.toml, CMake, scikit-build-core)
- [x] Qt type casters (QString, QByteArray, QList, QMap, QColor <-> Python)
- [x] QGuiApplication bootstrap on import
- [x] Distance type with arithmetic, conversions, comparisons
- [x] Point, Size, ColorNode, TextNode types
- [x] Units enum
- [x] Template database (1911 templates loaded)
- [x] Frame hierarchy (FrameRect, FrameRound, FrameEllipse, FrameCd, FrameContinuous, FramePath)
- [x] Layout, Paper, Category, Vendor types
- [x] Db static methods (lookup, query, register)
- [x] Model class with objects, templates, variables
- [x] ModelObject hierarchy (Text, Box, Ellipse, Line, Image, Barcode)
- [x] Variables and Variable with types and incrementing
- [x] File I/O (read_file, write_file, read_buffer, write_buffer)
- [x] Merge backends (CSV, TSV, Colon, Semicolon + keys variants)
- [x] Record with dict-like access
- [x] MergeFactory
- [x] PageRenderer with PDF rendering (12KB PDF generated successfully)
- [x] render_to_pdf() and render_model_to_pdf() convenience functions
- [x] BarcodeStyle and BarcodeBackends (36 styles)
- [x] Pythonic wrapper (Label class, template_db, batch fallback)
- [x] FontWeight, WrapMode, Alignment enums

## Tests: 31/31 PASSED

```
tests/python/test_distance.py       12 passed
tests/python/test_label.py           7 passed
tests/python/test_render.py          3 passed
tests/python/test_template_db.py     9 passed
```

## Known Issues

- [ ] **QThread warnings on exit**: Qt cleanup on exit produces warnings
      (`QThreadStorage: entry destroyed before end of thread`).
      Does not affect functionality.
- [ ] **Distance accessor naming**: Instance methods are `to_mm()`, `to_pt()` etc.
      (instead of `mm()`, `pt()`) due to conflict with static factory methods in pybind11.
- [ ] **Model/Object lifecycle**: Model and ModelObject use `py::nodelete` to
      avoid double-free with Qt parent-child ownership. Objects leak on exit,
      but this is acceptable for CLI/script use cases.

## Build Dependencies

The following are required to build:
```
sudo apt install qt6-base-dev qt6-svg-dev qt6-tools-dev qt6-l10n-tools \
  libcups2-dev zlib1g-dev python3-dev
pip install pybind11 scikit-build-core cmake
```

## Future Improvements

- [ ] Add `__init__.pyi` type stubs for IDE autocomplete
- [ ] CI/CD pipeline for automated builds and tests
- [ ] Wheel building for multiple platforms (cibuildwheel)
- [ ] More complete documentation with examples
- [ ] High-level Label.open() test with a real .glabels file
- [ ] Test for merge with CSV file
- [ ] Expose QTransform for advanced rotation/transform operations
- [ ] ModelEllipseObject full constructor
- [ ] Fix exit segfault (leak QGuiApplication or atexit handler)

## File Structure

```
glabels-qt-fork/
├── CMakeLists.txt                      # Main CMake (BUILD_PYTHON_BINDINGS option + fPIC)
├── pyproject.toml                      # pip install configuration (scikit-build-core)
├── model/                              # Core data model library
│   ├── CMakeLists.txt
│   ├── Distance.hpp/cpp                # Distance type
│   ├── Model.hpp/cpp                   # Model class
│   ├── ModelObject.hpp/cpp             # Base model object
│   ├── ModelTextObject.hpp/cpp         # Text, Box, Ellipse, Line, Image, Barcode objects
│   ├── Template.hpp/cpp                # Template, Frame*, Layout
│   ├── Db.hpp/cpp                      # Template database
│   ├── PageRenderer.hpp/cpp            # PDF rendering
│   ├── XmlLabelParser.hpp/cpp          # File I/O
│   ├── XmlLabelCreator.hpp/cpp
│   ├── Variable.hpp/cpp                # Variables
│   └── ...                             # Point, Size, Units, ColorNode, TextNode etc.
├── backends/
│   ├── CMakeLists.txt
│   ├── barcode/                        # Barcode backends
│   │   ├── CMakeLists.txt
│   │   ├── Backends.hpp/cpp            # BarcodeBackends (36 styles)
│   │   └── Style.hpp/cpp               # BarcodeStyle
│   └── merge/                          # Merge backends
│       ├── CMakeLists.txt
│       ├── Merge.hpp/cpp               # Base merge class
│       ├── Factory.hpp/cpp             # MergeFactory
│       ├── Record.hpp/cpp              # Record with dict-like access
│       └── Text*.hpp/cpp               # CSV, TSV, Colon, Semicolon + Keys variants
├── glbarcode/                          # Internal barcode library
├── glabels/                            # GUI application
├── glabels-batch/                      # Batch processing tool
├── python/
│   ├── TODO-python-bindings.md         # This file (Bulgarian)
│   ├── TODO-python-bindings-en.md      # This file (English)
│   ├── CMakeLists.txt                  # pybind11 module build
│   ├── glabels_ext.cpp                 # Module init + QApp bootstrap
│   ├── qt_type_casters.hpp             # QString/QList/QMap/QColor <-> Python
│   ├── bind_types.cpp                  # Distance, Point, Size, Units, ColorNode, TextNode
│   ├── bind_template.cpp              # Template, Frame*, Layout, Db, Paper, Category
│   ├── bind_model.cpp                  # Model, ModelObject*, Variables
│   ├── bind_io.cpp                     # XmlLabelParser, XmlLabelCreator (as functions)
│   ├── bind_renderer.cpp              # PageRenderer + render_to_pdf
│   ├── bind_merge.cpp                  # Merge, Record, MergeFactory
│   ├── bind_barcode.cpp               # BarcodeStyle, BarcodeBackends
│   └── glabels/                        # Pythonic wrapper package
│       ├── __init__.py                 # Auto-init, re-exports, mm()/cm()/inches()/pt()
│       ├── label.py                    # High-level Label class
│       ├── template_db.py             # find_template(), list_templates()
│       └── batch.py                    # Subprocess fallback
├── tests/
│   └── python/
│       ├── test_distance.py
│       ├── test_label.py
│       ├── test_template_db.py
│       └── test_render.py
└── templates/                          # 1911 label templates
```

## Usage Examples

### Loading from file

```python
import glabels

label = glabels.Label.open("my-labels.glabels")
label.render_pdf("output.pdf")
```

### Loading from XML string (in-memory)

```python
import glabels

xml = open("template.glabels", "rb").read()  # or from database, API, etc.
model = glabels.XmlLabelParser.read_buffer(xml)
glabels.render_model_to_pdf(model, "output.pdf")
```

### Serialization/deserialization in-memory

```python
import glabels

# Model -> bytes
buf = glabels.XmlLabelCreator.write_buffer(label.model)

# bytes -> Model
model = glabels.XmlLabelParser.read_buffer(buf)
```

### Creating a label programmatically

```python
import glabels

# Create from template
label = glabels.Label(template="Avery 5160")

# Add text
label.add_text(
    x=glabels.mm(2), y=glabels.mm(3),
    w=glabels.mm(60), h=glabels.mm(8),
    text="${name}", font_size=14,
)

# Add rectangle
label.add_box(
    x=glabels.mm(0), y=glabels.mm(0),
    w=glabels.mm(66), h=glabels.mm(25),
)

# Render to PDF
label.render_pdf("output.pdf", copies=30)

# Save as .glabels file
label.save("my-labels.glabels")
```

### I/O Reference

| Operation | Code |
|-----------|------|
| File -> Model | `model = glabels.XmlLabelParser.read_file("path.glabels")` |
| bytes -> Model | `model = glabels.XmlLabelParser.read_buffer(buf)` |
| Model -> file | `glabels.XmlLabelCreator.write_file(model, "path.glabels")` |
| Model -> bytes | `buf = glabels.XmlLabelCreator.write_buffer(model)` |
| High-level open | `label = glabels.Label.open("path.glabels")` |
| High-level save | `label.save("path.glabels")` |
| High-level PDF | `label.render_pdf("output.pdf")` |
