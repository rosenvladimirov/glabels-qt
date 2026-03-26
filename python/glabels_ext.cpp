#include <pybind11/pybind11.h>
#include "qt_type_casters.hpp"

#include <QGuiApplication>

#include "model/Db.hpp"
#include "model/Settings.hpp"
#include "model/Version.hpp"
#include "backends/barcode/Backends.hpp"
#include "backends/merge/Factory.hpp"

namespace py = pybind11;

// Forward declarations
void bind_types(py::module_& m);
void bind_template(py::module_& m);
void bind_model(py::module_& m);
void bind_io(py::module_& m);
void bind_renderer(py::module_& m);
void bind_merge(py::module_& m);
void bind_barcode(py::module_& m);


static QGuiApplication* s_qapp = nullptr;
static int    s_argc = 1;
static char   s_arg0[] = "glabels";
static char*  s_argv[] = { s_arg0, nullptr };


PYBIND11_MODULE(glabels_ext, m) {
    m.doc() = "gLabels-qt C++ bindings";

    m.def("init_all", []() {
        if (!QCoreApplication::instance()) {
            s_qapp = new QGuiApplication(s_argc, s_argv);
            QCoreApplication::setOrganizationName("glabels.org");
            QCoreApplication::setApplicationName("glabels-qt");
        }
        glabels::model::Settings::init();
        glabels::model::Db::init();
        glabels::barcode::Backends::init();
        glabels::merge::Factory::init();
    }, "Initialize Qt, template database, barcode backends, and merge backends");

    m.def("version", []() -> QString {
        return glabels::model::Version::STRING;
    }, "Get the gLabels version string");

    // Bind all types in order (dependencies first)
    bind_types(m);
    bind_template(m);
    bind_barcode(m);
    bind_merge(m);
    bind_model(m);
    bind_io(m);
    bind_renderer(m);
}
