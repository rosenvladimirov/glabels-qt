#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include "qt_type_casters.hpp"

#include "model/Template.hpp"
#include "model/Frame.hpp"
#include "model/FrameRect.hpp"
#include "model/FrameRound.hpp"
#include "model/FrameEllipse.hpp"
#include "model/FrameCd.hpp"
#include "model/FrameContinuous.hpp"
#include "model/FramePath.hpp"
#include "model/Layout.hpp"
#include "model/Db.hpp"
#include "model/Paper.hpp"
#include "model/Category.hpp"
#include "model/Vendor.hpp"

namespace py = pybind11;
using namespace glabels::model;


void bind_template(py::module_& m)
{
    // ---- Layout ----
    py::class_<Layout>(m, "Layout")
        .def(py::init<int, int, Distance, Distance, Distance, Distance>())
        .def("nx", &Layout::nx)
        .def("ny", &Layout::ny)
        .def("x0", &Layout::x0)
        .def("y0", &Layout::y0)
        .def("dx", &Layout::dx)
        .def("dy", &Layout::dy)
        .def("is_similar_to", &Layout::isSimilarTo);

    // ---- Frame (base, not directly constructible) ----
    py::class_<Frame>(m, "Frame")
        .def("id", &Frame::id)
        .def("n_labels", &Frame::nLabels)
        .def("layout_description", &Frame::layoutDescription)
        .def("w", &Frame::w)
        .def("h", &Frame::h);

    // ---- FrameRect ----
    py::class_<FrameRect, Frame>(m, "FrameRect")
        .def(py::init<Distance, Distance, Distance, Distance, Distance, const QString&>(),
             py::arg("w"), py::arg("h"), py::arg("r"),
             py::arg("x_waste"), py::arg("y_waste"), py::arg("id") = "0")
        .def("r", &FrameRect::r)
        .def("x_waste", &FrameRect::xWaste)
        .def("y_waste", &FrameRect::yWaste);

    // ---- Paper ----
    py::enum_<Paper::Type>(m, "PaperType")
        .value("SHEET", Paper::SHEET)
        .value("ENVELOPE", Paper::ENVELOPE)
        .value("ROLL", Paper::ROLL);

    py::class_<Paper>(m, "Paper")
        .def(py::init<>())
        .def("id", &Paper::id)
        .def("name", &Paper::name)
        .def("width", &Paper::width)
        .def("height", &Paper::height)
        .def("type", &Paper::type)
        .def("is_size_iso", &Paper::isSizeIso)
        .def("is_size_us", &Paper::isSizeUs)
        .def("__repr__", [](const Paper& p) {
            return "Paper('" + p.id().toStdString() + "', '" + p.name().toStdString() + "')";
        });

    // ---- Category ----
    py::class_<Category>(m, "Category")
        .def(py::init<>())
        .def("id", &Category::id)
        .def("name", &Category::name)
        .def("__repr__", [](const Category& c) {
            return "Category('" + c.id().toStdString() + "', '" + c.name().toStdString() + "')";
        });

    // ---- Vendor ----
    py::class_<Vendor>(m, "Vendor")
        .def(py::init<>())
        .def("name", &Vendor::name)
        .def("__repr__", [](const Vendor& v) {
            return "Vendor('" + v.name().toStdString() + "')";
        });

    // ---- Template ----
    py::class_<Template>(m, "Template")
        .def(py::init<>())
        .def("is_null", &Template::isNull)
        .def("brand", &Template::brand)
        .def("part", &Template::part)
        .def("description", &Template::description)
        .def("paper_id", &Template::paperId)
        .def("page_width", &Template::pageWidth)
        .def("page_height", &Template::pageHeight)
        .def("roll_width", &Template::rollWidth)
        .def("is_size_iso", &Template::isSizeIso)
        .def("is_size_us", &Template::isSizeUs)
        .def("is_roll", &Template::isRoll)
        .def("name", &Template::name)
        .def("has_category", &Template::hasCategory)
        .def("frame", &Template::frame,
             py::arg("id") = "0",
             py::return_value_policy::reference_internal)
        .def(py::self == py::self)
        .def("__repr__", [](const Template& t) {
            return "Template('" + t.name().toStdString() + "')";
        });

    // ---- Db (static-only class) ----
    py::class_<Db>(m, "Db")
        .def_static("init", &Db::init)
        .def_static("papers", &Db::papers)
        .def_static("paper_ids", &Db::paperIds)
        .def_static("paper_names", &Db::paperNames)
        .def_static("categories", &Db::categories)
        .def_static("category_ids", &Db::categoryIds)
        .def_static("category_names", &Db::categoryNames)
        .def_static("vendors", &Db::vendors)
        .def_static("vendor_names", &Db::vendorNames)
        .def_static("templates", &Db::templates)
        .def_static("lookup_paper_from_name", &Db::lookupPaperFromName)
        .def_static("lookup_paper_from_id", &Db::lookupPaperFromId)
        .def_static("lookup_paper_id_from_name", &Db::lookupPaperIdFromName)
        .def_static("lookup_paper_name_from_id", &Db::lookupPaperNameFromId)
        .def_static("lookup_category_from_name", &Db::lookupCategoryFromName)
        .def_static("lookup_category_from_id", &Db::lookupCategoryFromId)
        .def_static("lookup_template_from_name", &Db::lookupTemplateFromName)
        .def_static("lookup_template_from_brand_part", &Db::lookupTemplateFromBrandPart)
        .def_static("is_template_known", &Db::isTemplateKnown)
        .def_static("get_name_list_of_similar_templates", &Db::getNameListOfSimilarTemplates)
        .def_static("register_user_template", &Db::registerUserTemplate)
        .def_static("delete_user_template_by_brand_part", &Db::deleteUserTemplateByBrandPart);
}
