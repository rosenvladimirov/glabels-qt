#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include "qt_type_casters.hpp"

#include "backends/barcode/Style.hpp"
#include "backends/barcode/Backends.hpp"

namespace py = pybind11;
using namespace glabels::barcode;


void bind_barcode(py::module_& m)
{
    // ---- BarcodeStyle (barcode::Style) ----
    py::class_<Style>(m, "BarcodeStyle")
        .def(py::init<>())
        .def("id", &Style::id)
        .def("full_id", &Style::fullId)
        .def("backend_id", &Style::backendId)
        .def("name", &Style::name)
        .def("full_name", &Style::fullName)
        .def("can_text", &Style::canText)
        .def("text_optional", &Style::textOptional)
        .def("can_checksum", &Style::canChecksum)
        .def("checksum_optional", &Style::checksumOptional)
        .def("default_digits", &Style::defaultDigits)
        .def("can_freeform", &Style::canFreeform)
        .def("prefered_n", &Style::preferedN)
        .def("example_digits", &Style::exampleDigits)
        .def(py::self != py::self)
        .def("__repr__", [](const Style& s) {
            return "BarcodeStyle('" + s.fullName().toStdString() + "')";
        });

    // ---- BarcodeBackends (barcode::Backends) ----
    py::class_<Backends>(m, "BarcodeBackends")
        .def_static("init", &Backends::init)
        .def_static("backend_list", &Backends::backendList)
        .def_static("backend_name", &Backends::backendName)
        .def_static("style_list", &Backends::styleList)
        .def_static("default_style", &Backends::defaultStyle,
                     py::return_value_policy::reference)
        .def_static("style", &Backends::style,
                     py::return_value_policy::reference);
}
