#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include "qt_type_casters.hpp"

#include "model/Distance.hpp"
#include "model/Point.hpp"
#include "model/Size.hpp"
#include "model/Units.hpp"
#include "model/ColorNode.hpp"
#include "model/TextNode.hpp"

#include <QFont>
#include <QTextOption>

namespace py = pybind11;
using namespace glabels::model;


void bind_types(py::module_& m)
{
    // ---- Units::Enum ----
    py::enum_<Units::Enum>(m, "UnitsEnum")
        .value("PT", Units::PT)
        .value("IN", Units::IN)
        .value("MM", Units::MM)
        .value("CM", Units::CM)
        .value("PC", Units::PC);

    // ---- Units ----
    py::class_<Units>(m, "Units")
        .def(py::init<>())
        .def(py::init<Units::Enum>())
        .def(py::init<const QString&>())
        .def_static("pt", &Units::pt)
        .def_static("in_", &Units::in)
        .def_static("mm", &Units::mm)
        .def_static("cm", &Units::cm)
        .def_static("pc", &Units::pc)
        .def("to_enum", &Units::toEnum)
        .def("to_id_string", &Units::toIdString)
        .def("to_tr_name", &Units::toTrName)
        .def("resolution", &Units::resolution)
        .def("resolution_digits", &Units::resolutionDigits);

    // ---- Distance ----
    // Static factories are lambdas to avoid overload ambiguity
    py::class_<Distance>(m, "Distance")
        .def(py::init<>())
        .def(py::init<double, Units::Enum>(), py::arg("d"), py::arg("units") = Units::PT)
        .def_static("pt", [](double v) { return Distance::pt(v); })
        .def_static("in_", [](double v) { return Distance::in(v); })
        .def_static("mm", [](double v) { return Distance::mm(v); })
        .def_static("cm", [](double v) { return Distance::cm(v); })
        .def_static("pc", [](double v) { return Distance::pc(v); })
        .def("to_pt", [](const Distance& d) { return d.pt(); })
        .def("to_in", [](const Distance& d) { return d.in(); })
        .def("to_mm", [](const Distance& d) { return d.mm(); })
        .def("to_cm", [](const Distance& d) { return d.cm(); })
        .def("to_pc", [](const Distance& d) { return d.pc(); })
        .def("in_units", [](const Distance& d, Units::Enum e) { return d.inUnits(e); })
        .def(py::self + py::self)
        .def(py::self - py::self)
        .def(py::self * double())
        .def(double() * py::self)
        .def(py::self / double())
        .def("__truediv__", [](Distance d1, Distance d2) { return d1 / d2; })
        .def("__neg__", [](Distance d) { return -d; })
        .def(py::self += py::self)
        .def(py::self -= py::self)
        .def(py::self *= double())
        .def(py::self < py::self)
        .def(py::self <= py::self)
        .def(py::self > py::self)
        .def(py::self >= py::self)
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def("__repr__", [](const Distance& d) {
            return "Distance(" + std::to_string(d.pt()) + " pt)";
        })
        .def("__float__", [](const Distance& d) { return d.pt(); });

    // ---- Point ----
    py::class_<Point>(m, "Point")
        .def(py::init<>())
        .def(py::init<Distance, Distance>())
        .def("x", &Point::x)
        .def("y", &Point::y)
        .def("__repr__", [](const Point& p) {
            return "Point(" + std::to_string(p.x().pt()) + ", " + std::to_string(p.y().pt()) + ")";
        });

    // ---- Size ----
    py::class_<Size>(m, "Size")
        .def(py::init<>())
        .def(py::init<Distance, Distance>())
        .def("w", &Size::w)
        .def("h", &Size::h)
        .def("set_w", &Size::setW)
        .def("set_h", &Size::setH)
        .def("__repr__", [](const Size& s) {
            return "Size(" + std::to_string(s.w().pt()) + ", " + std::to_string(s.h().pt()) + ")";
        });

    // ---- ColorNode ----
    // operators are non-const in C++, use lambdas
    py::class_<ColorNode>(m, "ColorNode")
        .def(py::init<>())
        .def(py::init<const QColor&>())
        .def(py::init<const QString&>())
        .def(py::init<bool, const QColor&, const QString&>())
        .def("is_field", &ColorNode::isField)
        .def("set_field", &ColorNode::setField)
        .def("color", py::overload_cast<>(&ColorNode::color, py::const_))
        .def("set_color", &ColorNode::setColor)
        .def("key", &ColorNode::key)
        .def("set_key", &ColorNode::setKey)
        .def("rgba", &ColorNode::rgba)
        .def("__eq__", [](ColorNode& a, ColorNode& b) { return a == b; })
        .def("__ne__", [](ColorNode& a, ColorNode& b) { return a != b; });

    // ---- TextNode ----
    py::class_<TextNode>(m, "TextNode")
        .def(py::init<>())
        .def(py::init<bool, const QString&>())
        .def("is_field", &TextNode::isField)
        .def("set_field", &TextNode::setField)
        .def("data", &TextNode::data)
        .def("set_data", &TextNode::setData)
        .def("__eq__", [](TextNode& a, TextNode& b) { return a == b; })
        .def("__ne__", [](TextNode& a, TextNode& b) { return a != b; });

    // ---- FontWeight enum (QFont::Weight) ----
    py::enum_<QFont::Weight>(m, "FontWeight")
        .value("Thin", QFont::Thin)
        .value("ExtraLight", QFont::ExtraLight)
        .value("Light", QFont::Light)
        .value("Normal", QFont::Normal)
        .value("Medium", QFont::Medium)
        .value("DemiBold", QFont::DemiBold)
        .value("Bold", QFont::Bold)
        .value("ExtraBold", QFont::ExtraBold)
        .value("Black", QFont::Black);

    // ---- WrapMode enum (QTextOption::WrapMode) ----
    py::enum_<QTextOption::WrapMode>(m, "WrapMode")
        .value("NoWrap", QTextOption::NoWrap)
        .value("WordWrap", QTextOption::WordWrap)
        .value("ManualWrap", QTextOption::ManualWrap)
        .value("WrapAnywhere", QTextOption::WrapAnywhere)
        .value("WrapAtWordBoundaryOrAnywhere", QTextOption::WrapAtWordBoundaryOrAnywhere);

    // ---- Qt::Alignment constants ----
    m.attr("AlignLeft")    = py::int_(static_cast<int>(Qt::AlignLeft));
    m.attr("AlignRight")   = py::int_(static_cast<int>(Qt::AlignRight));
    m.attr("AlignHCenter") = py::int_(static_cast<int>(Qt::AlignHCenter));
    m.attr("AlignTop")     = py::int_(static_cast<int>(Qt::AlignTop));
    m.attr("AlignBottom")  = py::int_(static_cast<int>(Qt::AlignBottom));
    m.attr("AlignVCenter") = py::int_(static_cast<int>(Qt::AlignVCenter));
}
