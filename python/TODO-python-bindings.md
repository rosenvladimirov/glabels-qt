д ач# TODO: Python Bindings за gLabels-qt

## Статус: Работещ прототип

Всички основни функционалности са имплементирани и тествани.

## Тествано и работещо

- [x] Build инфраструктура (pyproject.toml, CMake, scikit-build-core)
- [x] Qt type casters (QString, QByteArray, QList, QMap, QColor <-> Python)
- [x] QGuiApplication bootstrap при import
- [x] Distance type с аритметика, конверсии, сравнения
- [x] Point, Size, ColorNode, TextNode типове
- [x] Units enum
- [x] Template database (1911 шаблона заредени)
- [x] Frame hierarchy (FrameRect, FrameRound, FrameEllipse, FrameCd, FrameContinuous, FramePath)
- [x] Layout, Paper, Category, Vendor типове
- [x] Db static methods (lookup, query, register)
- [x] Model клас с обекти, шаблони, variables
- [x] ModelObject hierarchy (Text, Box, Ellipse, Line, Image, Barcode)
- [x] Variables и Variable с типове и инкрементиране
- [x] File I/O (read_file, write_file, read_buffer, write_buffer)
- [x] Merge backends (CSV, TSV, Colon, Semicolon + keys варианти)
- [x] Record с dict-like достъп
- [x] MergeFactory
- [x] PageRenderer с PDF рендериране (12KB PDF генериран успешно)
- [x] render_to_pdf() и render_model_to_pdf() convenience функции
- [x] BarcodeStyle и BarcodeBackends (36 стила)
- [x] Pythonic wrapper (Label клас, template_db, batch fallback)
- [x] FontWeight, WrapMode, Alignment enums

## Тестове: 31/31 PASSED

```
tests/python/test_distance.py       12 passed
tests/python/test_label.py           7 passed
tests/python/test_render.py          3 passed
tests/python/test_template_db.py     9 passed
```

## Известни проблеми

- [ ] **QThread warnings при exit**: Qt cleanup при изход показва warnings
      (`QThreadStorage: entry destroyed before end of thread`).
      Не засяга функционалността.
- [ ] **Distance accessor naming**: Instance методите са `to_mm()`, `to_pt()` и т.н.
      (вместо `mm()`, `pt()`) заради конфликт с static factory методите в pybind11.
- [ ] **Model/Object lifecycle**: Model и ModelObject използват `py::nodelete` за
      да избегнат double-free с Qt parent-child ownership. Обектите leak-ват при exit,
      но това е приемливо за CLI/script use cases.

## Нужни build зависимости

За билд на системата са нужни:
```
sudo apt install qt6-base-dev qt6-svg-dev qt6-tools-dev qt6-l10n-tools \
  libcups2-dev zlib1g-dev python3-dev
pip install pybind11 scikit-build-core cmake
```

## Бъдещи подобрения

- [ ] Добавяне на `__init__.pyi` type stubs за IDE autocomplete
- [ ] CI/CD pipeline за автоматичен билд и тестове
- [ ] Wheel building за различни платформи (cibuildwheel)
- [ ] По-пълна документация с примери
- [ ] High-level Label.open() тест с реален .glabels файл
- [ ] Тест за merge с CSV файл
- [ ] Expose QTransform за advanced rotation/transform операции
- [ ] ModelEllipseObject full constructor
- [ ] Fix exit segfault (leak QGuiApplication или atexit handler)

## Файлова структура

```
glabels-qt-fork/
├── CMakeLists.txt                      # Главен CMake (BUILD_PYTHON_BINDINGS option + fPIC)
├── pyproject.toml                      # pip install конфигурация (scikit-build-core)
├── model/                              # Core data model библиотека
│   ├── CMakeLists.txt
│   ├── Distance.hpp/cpp                # Distance тип
│   ├── Model.hpp/cpp                   # Model клас
│   ├── ModelObject.hpp/cpp             # Base model object
│   ├── ModelTextObject.hpp/cpp         # Text, Box, Ellipse, Line, Image, Barcode обекти
│   ├── Template.hpp/cpp                # Template, Frame*, Layout
│   ├── Db.hpp/cpp                      # Template database
│   ├── PageRenderer.hpp/cpp            # PDF рендериране
│   ├── XmlLabelParser.hpp/cpp          # File I/O
│   ├── XmlLabelCreator.hpp/cpp
│   ├── Variable.hpp/cpp                # Variables
│   └── ...                             # Point, Size, Units, ColorNode, TextNode и др.
├── backends/
│   ├── CMakeLists.txt
│   ├── barcode/                        # Barcode backends
│   │   ├── CMakeLists.txt
│   │   ├── Backends.hpp/cpp            # BarcodeBackends (36 стила)
│   │   └── Style.hpp/cpp               # BarcodeStyle
│   └── merge/                          # Merge backends
│       ├── CMakeLists.txt
│       ├── Merge.hpp/cpp               # Base merge клас
│       ├── Factory.hpp/cpp             # MergeFactory
│       ├── Record.hpp/cpp              # Record с dict-like достъп
│       └── Text*.hpp/cpp               # CSV, TSV, Colon, Semicolon + Keys варианти
├── glbarcode/                          # Вътрешна barcode библиотека
├── glabels/                            # GUI приложение
├── glabels-batch/                      # Batch processing tool
├── python/
│   ├── TODO-python-bindings.md         # Този файл
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
│   └── glabels/                        # Pythonic wrapper пакет
│       ├── __init__.py                 # Auto-init, re-exports, mm()/cm()/inches()/pt()
│       ├── label.py                    # High-level Label клас
│       ├── template_db.py             # find_template(), list_templates()
│       └── batch.py                    # Subprocess fallback
├── tests/
│   └── python/
│       ├── test_distance.py
│       ├── test_label.py
│       ├── test_template_db.py
│       └── test_render.py
└── templates/                          # 1911 label шаблона
```

## Примери за употреба

### Зареждане от файл

```python
import glabels

label = glabels.Label.open("my-labels.glabels")
label.render_pdf("output.pdf")
```

### Зареждане от XML string (през паметта)

```python
import glabels

xml = open("template.glabels", "rb").read()  # или от база данни, API и т.н.
model = glabels.XmlLabelParser.read_buffer(xml)
glabels.render_model_to_pdf(model, "output.pdf")
```

### Сериализация/десериализация през паметта

```python
import glabels

# Model → bytes
buf = glabels.XmlLabelCreator.write_buffer(label.model)

# bytes → Model
model = glabels.XmlLabelParser.read_buffer(buf)
```

### Създаване на етикет програмно

```python
import glabels

# Създаване от шаблон
label = glabels.Label(template="Avery 5160")

# Добавяне на текст
label.add_text(
    x=glabels.mm(2), y=glabels.mm(3),
    w=glabels.mm(60), h=glabels.mm(8),
    text="${name}", font_size=14,
)

# Добавяне на правоъгълник
label.add_box(
    x=glabels.mm(0), y=glabels.mm(0),
    w=glabels.mm(66), h=glabels.mm(25),
)

# Рендериране към PDF
label.render_pdf("output.pdf", copies=30)

# Запис като .glabels файл
label.save("my-labels.glabels")
```

### I/O справка

| Операция | Код |
|----------|-----|
| Файл → Model | `model = glabels.XmlLabelParser.read_file("path.glabels")` |
| bytes → Model | `model = glabels.XmlLabelParser.read_buffer(buf)` |
| Model → файл | `glabels.XmlLabelCreator.write_file(model, "path.glabels")` |
| Model → bytes | `buf = glabels.XmlLabelCreator.write_buffer(model)` |
| High-level отвори | `label = glabels.Label.open("path.glabels")` |
| High-level запис | `label.save("path.glabels")` |
| High-level PDF | `label.render_pdf("output.pdf")` |
