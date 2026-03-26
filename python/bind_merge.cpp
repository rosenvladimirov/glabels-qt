#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "qt_type_casters.hpp"

#include "backends/merge/Merge.hpp"
#include "backends/merge/Record.hpp"
#include "backends/merge/Factory.hpp"

namespace py = pybind11;
using namespace glabels::merge;


void bind_merge(py::module_& m)
{
    // ---- MergeSourceType (Factory::SourceType) ----
    py::enum_<Factory::SourceType>(m, "MergeSourceType")
        .value("NONE", Factory::NONE)
        .value("FIXED", Factory::FIXED)
        .value("FILE", Factory::FILE);

    // ---- Record (dict-like) ----
    py::class_<Record>(m, "Record")
        .def(py::init<>())
        .def("is_selected", &Record::isSelected)
        .def("set_selected", &Record::setSelected)
        .def("__len__", &Record::size)
        .def("__contains__", [](const Record& r, const QString& key) {
            return r.contains(key);
        })
        .def("__getitem__", [](const Record& r, const QString& key) -> QString {
            if (!r.contains(key))
                throw py::key_error("Key not found: " + key.toStdString());
            return r.value(key);
        })
        .def("__setitem__", [](Record& r, const QString& key, const QString& value) {
            r.insert(key, value);
        })
        .def("keys", [](const Record& r) {
            return r.keys();
        })
        .def("values", [](const Record& r) {
            QStringList vals;
            for (auto it = r.constBegin(); it != r.constEnd(); ++it)
                vals.append(it.value());
            return vals;
        })
        .def("items", [](const Record& r) {
            py::list result;
            for (auto it = r.constBegin(); it != r.constEnd(); ++it)
                result.append(py::make_tuple(it.key(), it.value()));
            return result;
        })
        .def("__repr__", [](const Record& r) {
            std::string s = "Record({";
            bool first = true;
            for (auto it = r.constBegin(); it != r.constEnd(); ++it) {
                if (!first) s += ", ";
                s += "'" + it.key().toStdString() + "': '" + it.value().toStdString() + "'";
                first = false;
            }
            s += "})";
            return s;
        });

    // ---- Merge (QObject, py::nodelete) ----
    py::class_<Merge, std::unique_ptr<Merge, py::nodelete>>(m, "Merge")
        .def("id", &Merge::id)
        .def("source", &Merge::source)
        .def("set_source", &Merge::setSource)
        .def("reload_source", &Merge::reloadSource)
        .def("record_list", &Merge::recordList)
        .def("set_selected", &Merge::setSelected)
        .def("select_all", &Merge::selectAll)
        .def("unselect_all", &Merge::unselectAll)
        .def("n_selected_records", &Merge::nSelectedRecords)
        .def("selected_records", &Merge::selectedRecords)
        .def("keys", &Merge::keys)
        .def("primary_key", &Merge::primaryKey);

    // ---- MergeFactory (Factory) ----
    py::class_<Factory>(m, "MergeFactory")
        .def_static("init", &Factory::init)
        .def_static("create_merge", &Factory::createMerge,
                     py::return_value_policy::take_ownership)
        .def_static("name_list", &Factory::nameList)
        .def_static("id_to_name", &Factory::idToName)
        .def_static("name_to_id", &Factory::nameToId)
        .def_static("id_to_type", &Factory::idToType)
        .def_static("index_to_id", &Factory::indexToId);
}
