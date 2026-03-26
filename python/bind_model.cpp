#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "qt_type_casters.hpp"

#include "model/Model.hpp"
#include "model/Size.hpp"
#include "model/ModelObject.hpp"
#include "model/ModelTextObject.hpp"
#include "model/ModelBoxObject.hpp"
#include "model/ModelEllipseObject.hpp"
#include "model/ModelLineObject.hpp"
#include "model/ModelImageObject.hpp"
#include "model/ModelBarcodeObject.hpp"
#include "model/ModelShapeObject.hpp"
#include "model/Variable.hpp"
#include "model/Variables.hpp"

namespace py = pybind11;
using namespace glabels::model;
using glabels::barcode::Style;


void bind_model(py::module_& m)
{
    // ---- Variable::Type ----
    py::enum_<Variable::Type>(m, "VariableType")
        .value("STRING", Variable::Type::STRING)
        .value("INTEGER", Variable::Type::INTEGER)
        .value("FLOATING_POINT", Variable::Type::FLOATING_POINT)
        .value("COLOR", Variable::Type::COLOR);

    // ---- Variable::Increment ----
    py::enum_<Variable::Increment>(m, "VariableIncrement")
        .value("NEVER", Variable::Increment::NEVER)
        .value("PER_ITEM", Variable::Increment::PER_ITEM)
        .value("PER_COPY", Variable::Increment::PER_COPY)
        .value("PER_PAGE", Variable::Increment::PER_PAGE);

    // ---- Variable ----
    py::class_<Variable>(m, "Variable")
        .def(py::init<>())
        .def(py::init<Variable::Type, const QString&, const QString&, Variable::Increment, const QString&>(),
             py::arg("type"), py::arg("name"), py::arg("initial_value"),
             py::arg("increment") = Variable::Increment::NEVER,
             py::arg("step_size") = "0")
        .def("type", &Variable::type)
        .def("name", &Variable::name)
        .def("initial_value", &Variable::initialValue)
        .def("increment", &Variable::increment)
        .def("step_size", &Variable::stepSize)
        .def("set_initial_value", &Variable::setInitialValue)
        .def("reset_value", &Variable::resetValue)
        .def("value", &Variable::value);

    // ---- Variables (QObject, py::nodelete) ----
    py::class_<Variables>(m, "Variables")
        .def(py::init<>())
        .def("has_variable", &Variables::hasVariable)
        .def("add_variable", &Variables::addVariable)
        .def("delete_variable", &Variables::deleteVariable)
        .def("replace_variable", &Variables::replaceVariable)
        .def("set_variables", &Variables::setVariables)
        .def("reset_variables", &Variables::resetVariables)
        .def("__getitem__", [](const Variables& v, const QString& key) -> Variable {
            if (!v.contains(key))
                throw py::key_error("Variable not found: " + key.toStdString());
            return v.value(key);
        })
        .def("__contains__", [](const Variables& v, const QString& key) {
            return v.contains(key);
        })
        .def("__len__", &Variables::size);

    // ---- ModelObject (abstract base, py::nodelete) ----
    py::class_<ModelObject, std::unique_ptr<ModelObject, py::nodelete>>(m, "ModelObject")
        .def("id", &ModelObject::id)
        .def("x0", &ModelObject::x0)
        .def("y0", &ModelObject::y0)
        .def("w", &ModelObject::w)
        .def("h", &ModelObject::h)
        .def("set_x0", &ModelObject::setX0)
        .def("set_y0", &ModelObject::setY0)
        .def("set_w", &ModelObject::setW)
        .def("set_h", &ModelObject::setH)
        .def("lock_aspect_ratio", &ModelObject::lockAspectRatio)
        .def("set_lock_aspect_ratio", &ModelObject::setLockAspectRatio)
        .def("shadow", &ModelObject::shadow)
        .def("set_shadow", &ModelObject::setShadow)
        .def("shadow_x", &ModelObject::shadowX)
        .def("set_shadow_x", &ModelObject::setShadowX)
        .def("shadow_y", &ModelObject::shadowY)
        .def("set_shadow_y", &ModelObject::setShadowY)
        .def("shadow_opacity", &ModelObject::shadowOpacity)
        .def("set_shadow_opacity", &ModelObject::setShadowOpacity)
        .def("natural_size", &ModelObject::naturalSize)
        // Text properties (virtual)
        .def("text", &ModelObject::text)
        .def("set_text", &ModelObject::setText)
        .def("font_family", &ModelObject::fontFamily)
        .def("set_font_family", &ModelObject::setFontFamily)
        .def("font_size", &ModelObject::fontSize)
        .def("set_font_size", &ModelObject::setFontSize)
        .def("font_weight", &ModelObject::fontWeight)
        .def("set_font_weight", &ModelObject::setFontWeight)
        .def("font_italic_flag", &ModelObject::fontItalicFlag)
        .def("set_font_italic_flag", &ModelObject::setFontItalicFlag)
        .def("font_underline_flag", &ModelObject::fontUnderlineFlag)
        .def("set_font_underline_flag", &ModelObject::setFontUnderlineFlag)
        .def("text_color_node", &ModelObject::textColorNode)
        .def("set_text_color_node", &ModelObject::setTextColorNode)
        .def("text_h_align", [](const ModelObject& o) { return static_cast<int>(o.textHAlign()); })
        .def("set_text_h_align", [](ModelObject& o, int v) { o.setTextHAlign(static_cast<Qt::Alignment>(v)); })
        .def("text_v_align", [](const ModelObject& o) { return static_cast<int>(o.textVAlign()); })
        .def("set_text_v_align", [](ModelObject& o, int v) { o.setTextVAlign(static_cast<Qt::Alignment>(v)); })
        .def("text_wrap_mode", &ModelObject::textWrapMode)
        .def("set_text_wrap_mode", &ModelObject::setTextWrapMode)
        .def("text_line_spacing", &ModelObject::textLineSpacing)
        .def("set_text_line_spacing", &ModelObject::setTextLineSpacing)
        .def("text_auto_shrink", &ModelObject::textAutoShrink)
        .def("set_text_auto_shrink", &ModelObject::setTextAutoShrink)
        // Image properties (virtual)
        .def("filename_node", &ModelObject::filenameNode)
        .def("set_filename_node", &ModelObject::setFilenameNode)
        // Shape properties (virtual)
        .def("line_width", &ModelObject::lineWidth)
        .def("set_line_width", &ModelObject::setLineWidth)
        .def("line_color_node", &ModelObject::lineColorNode)
        .def("set_line_color_node", &ModelObject::setLineColorNode)
        .def("fill_color_node", &ModelObject::fillColorNode)
        .def("set_fill_color_node", &ModelObject::setFillColorNode)
        // Barcode properties (virtual)
        .def("bc_data", &ModelObject::bcData)
        .def("set_bc_data", &ModelObject::setBcData)
        .def("bc_text_flag", &ModelObject::bcTextFlag)
        .def("set_bc_text_flag", &ModelObject::setBcTextFlag)
        .def("bc_checksum_flag", &ModelObject::bcChecksumFlag)
        .def("set_bc_checksum_flag", &ModelObject::setBcChecksumFlag)
        .def("bc_color_node", &ModelObject::bcColorNode)
        .def("set_bc_color_node", &ModelObject::setBcColorNode)
        .def("bc_style", &ModelObject::bcStyle)
        .def("set_bc_style", &ModelObject::setBcStyle)
        // Capabilities
        .def("can_text", &ModelObject::canText)
        .def("can_fill", &ModelObject::canFill)
        .def("can_line_color", &ModelObject::canLineColor)
        .def("can_line_width", &ModelObject::canLineWidth);

    // ---- ModelTextObject ----
    py::class_<ModelTextObject, ModelObject, std::unique_ptr<ModelTextObject, py::nodelete>>(m, "ModelTextObject")
        .def(py::init<>())
        .def(py::init([](Distance x0, Distance y0, Distance w, Distance h,
                         bool lockAspectRatio, const QString& text,
                         const QString& fontFamily, double fontSize,
                         QFont::Weight fontWeight, bool fontItalicFlag,
                         bool fontUnderlineFlag, ColorNode textColorNode,
                         int textHAlign, int textVAlign,
                         QTextOption::WrapMode textWrapMode,
                         double textLineSpacing, bool textAutoShrink) {
                 return new ModelTextObject(
                     x0, y0, w, h, lockAspectRatio, text,
                     fontFamily, fontSize, fontWeight,
                     fontItalicFlag, fontUnderlineFlag, textColorNode,
                     static_cast<Qt::Alignment>(textHAlign),
                     static_cast<Qt::Alignment>(textVAlign),
                     textWrapMode, textLineSpacing, textAutoShrink);
             }),
             py::arg("x0"), py::arg("y0"), py::arg("w"), py::arg("h"),
             py::arg("lock_aspect_ratio"),
             py::arg("text"), py::arg("font_family"), py::arg("font_size"),
             py::arg("font_weight"), py::arg("font_italic_flag"),
             py::arg("font_underline_flag"), py::arg("text_color_node"),
             py::arg("text_h_align"), py::arg("text_v_align"),
             py::arg("text_wrap_mode"), py::arg("text_line_spacing"),
             py::arg("text_auto_shrink"));

    // ---- ModelShapeObject (abstract base for Box/Ellipse) ----
    py::class_<ModelShapeObject, ModelObject, std::unique_ptr<ModelShapeObject, py::nodelete>>(m, "ModelShapeObject");

    // ---- ModelBoxObject ----
    py::class_<ModelBoxObject, ModelShapeObject, std::unique_ptr<ModelBoxObject, py::nodelete>>(m, "ModelBoxObject")
        .def(py::init<>())
        .def(py::init<Distance, Distance, Distance, Distance,
                       bool, Distance, const ColorNode&, const ColorNode&>(),
             py::arg("x0"), py::arg("y0"), py::arg("w"), py::arg("h"),
             py::arg("lock_aspect_ratio"),
             py::arg("line_width"), py::arg("line_color_node"),
             py::arg("fill_color_node"));

    // ---- ModelEllipseObject ----
    py::class_<ModelEllipseObject, ModelShapeObject, std::unique_ptr<ModelEllipseObject, py::nodelete>>(m, "ModelEllipseObject")
        .def(py::init<>())
        .def(py::init<Distance, Distance, Distance, Distance,
                       bool, Distance, const ColorNode&, const ColorNode&>(),
             py::arg("x0"), py::arg("y0"), py::arg("w"), py::arg("h"),
             py::arg("lock_aspect_ratio"),
             py::arg("line_width"), py::arg("line_color_node"),
             py::arg("fill_color_node"));

    // ---- ModelLineObject ----
    py::class_<ModelLineObject, ModelObject, std::unique_ptr<ModelLineObject, py::nodelete>>(m, "ModelLineObject")
        .def(py::init<>())
        .def(py::init<Distance, Distance, Distance, Distance,
                       Distance, const ColorNode&>(),
             py::arg("x0"), py::arg("y0"), py::arg("w"), py::arg("h"),
             py::arg("line_width"), py::arg("line_color_node"));

    // ---- ModelImageObject ----
    py::class_<ModelImageObject, ModelObject, std::unique_ptr<ModelImageObject, py::nodelete>>(m, "ModelImageObject")
        .def(py::init<>());

    // ---- ModelBarcodeObject ----
    py::class_<ModelBarcodeObject, ModelObject, std::unique_ptr<ModelBarcodeObject, py::nodelete>>(m, "ModelBarcodeObject")
        .def(py::init<>())
        .def(py::init<Distance, Distance, Distance, Distance,
                       bool, const Style&, bool, bool,
                       QString, const ColorNode&>(),
             py::arg("x0"), py::arg("y0"), py::arg("w"), py::arg("h"),
             py::arg("lock_aspect_ratio"),
             py::arg("bc_style"), py::arg("bc_text_flag"),
             py::arg("bc_checksum_flag"), py::arg("bc_data"),
             py::arg("bc_color_node"));

    // ---- Model (QObject, py::nodelete) ----
    py::class_<Model, std::unique_ptr<Model, py::nodelete>>(m, "Model")
        .def(py::init<>())
        .def("is_modified", &Model::isModified)
        .def("set_modified", &Model::setModified)
        .def("clear_modified", &Model::clearModified)
        .def("short_name", &Model::shortName)
        .def("file_name", &Model::fileName)
        .def("set_file_name", &Model::setFileName)
        .def("tmplate", &Model::tmplate)
        .def("set_tmplate", &Model::setTmplate)
        .def("frame", &Model::frame,
             py::arg("id") = "0",
             py::return_value_policy::reference_internal)
        .def("rotate", &Model::rotate)
        .def("set_rotate", &Model::setRotate)
        .def("w", &Model::w)
        .def("h", &Model::h)
        .def("object_list", &Model::objectList,
             py::return_value_policy::reference_internal)
        .def("variables", py::overload_cast<>(&Model::variables),
             py::return_value_policy::reference_internal)
        .def("merge", &Model::merge,
             py::return_value_policy::reference_internal)
        .def("set_merge", &Model::setMerge)
        .def("add_object", &Model::addObject)
        .def("delete_object", &Model::deleteObject)
        .def("is_selection_empty", &Model::isSelectionEmpty)
        .def("select_all", &Model::selectAll)
        .def("unselect_all", &Model::unselectAll);
}
