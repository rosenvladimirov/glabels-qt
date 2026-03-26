#include <pybind11/pybind11.h>
#include "qt_type_casters.hpp"

#include "model/XmlLabelParser.hpp"
#include "model/XmlLabelCreator.hpp"
#include "model/Model.hpp"

namespace py = pybind11;
using namespace glabels::model;


void bind_io(py::module_& m)
{
    // Expose as simple struct with static methods (no QObject typeinfo needed)
    struct XmlLabelParserWrapper {};
    py::class_<XmlLabelParserWrapper>(m, "XmlLabelParser")
        .def_static("read_file", &XmlLabelParser::readFile,
                     py::arg("filename"),
                     py::return_value_policy::take_ownership)
        .def_static("read_buffer", &XmlLabelParser::readBuffer,
                     py::arg("buffer"),
                     py::return_value_policy::take_ownership);

    struct XmlLabelCreatorWrapper {};
    py::class_<XmlLabelCreatorWrapper>(m, "XmlLabelCreator")
        .def_static("write_file", &XmlLabelCreator::writeFile,
                     py::arg("model"), py::arg("filename"))
        .def_static("write_buffer", [](const Model* model) {
            QByteArray buffer;
            XmlLabelCreator::writeBuffer(model, buffer);
            return buffer;
        }, py::arg("model"));

    // Module-level convenience functions
    m.def("read_file", &XmlLabelParser::readFile,
          py::arg("filename"),
          py::return_value_policy::take_ownership);

    m.def("write_file", &XmlLabelCreator::writeFile,
          py::arg("model"), py::arg("filename"));

    // io submodule
    auto io = m.def_submodule("io", "File I/O functions");
    io.def("read_file", &XmlLabelParser::readFile,
           py::arg("filename"),
           py::return_value_policy::take_ownership);
    io.def("read_buffer", &XmlLabelParser::readBuffer,
           py::arg("buffer"),
           py::return_value_policy::take_ownership);
    io.def("write_file", &XmlLabelCreator::writeFile,
           py::arg("model"), py::arg("filename"));
    io.def("write_buffer", [](const Model* model) {
        QByteArray buffer;
        XmlLabelCreator::writeBuffer(model, buffer);
        return buffer;
    }, py::arg("model"));
}
