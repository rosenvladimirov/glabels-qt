#include <pybind11/pybind11.h>
#include "qt_type_casters.hpp"

#include "model/PageRenderer.hpp"
#include "model/Model.hpp"

#include <QPdfWriter>
#include <QPainter>

namespace py = pybind11;
using namespace glabels::model;


static void render_to_pdf_impl(PageRenderer& renderer, const QString& filename)
{
    const Model* model = renderer.model();
    if (!model) {
        throw std::runtime_error("PageRenderer has no model set");
    }

    const Frame* frame = model->frame();
    if (!frame) {
        throw std::runtime_error("Model has no frame (no template set)");
    }

    QPdfWriter pdfWriter(filename);
    pdfWriter.setResolution(72);

    QPageSize pageSize(QSizeF(model->tmplate().pageWidth().pt(),
                               model->tmplate().pageHeight().pt()),
                       QPageSize::Point);
    pdfWriter.setPageSize(pageSize);
    pdfWriter.setPageMargins(QMarginsF(0, 0, 0, 0));

    QPainter painter(&pdfWriter);
    int nPages = renderer.nPages();
    for (int i = 0; i < nPages; i++) {
        if (i > 0) {
            pdfWriter.newPage();
        }
        renderer.printPage(&painter, i);
    }
    painter.end();
}


void bind_renderer(py::module_& m)
{
    // ---- PageRenderer (QObject, py::nodelete) ----
    py::class_<PageRenderer, std::unique_ptr<PageRenderer, py::nodelete>>(m, "PageRenderer")
        .def(py::init<const Model*>(), py::arg("model") = nullptr)
        .def("set_model", &PageRenderer::setModel)
        .def("model", &PageRenderer::model, py::return_value_policy::reference)
        .def("set_n_copies", &PageRenderer::setNCopies)
        .def("set_start_item", &PageRenderer::setStartItem)
        .def("set_is_collated", &PageRenderer::setIsCollated)
        .def("set_are_groups_contiguous", &PageRenderer::setAreGroupsContiguous)
        .def("set_print_outlines", &PageRenderer::setPrintOutlines)
        .def("set_print_crop_marks", &PageRenderer::setPrintCropMarks)
        .def("set_print_reverse", &PageRenderer::setPrintReverse)
        .def("n_items", &PageRenderer::nItems)
        .def("n_pages", &PageRenderer::nPages);

    // ---- render_to_pdf convenience function ----
    m.def("render_to_pdf", &render_to_pdf_impl,
          py::arg("renderer"), py::arg("filename"),
          "Render pages to a PDF file using a configured PageRenderer");

    // ---- render_model_to_pdf convenience function ----
    m.def("render_model_to_pdf",
          [](Model* model, const QString& filename,
             int copies, int startItem,
             bool outlines, bool cropMarks,
             bool reverse, bool collated,
             bool groupsContiguous) {
              PageRenderer renderer(model);

              const Frame* frame = model->frame();
              int nLabels = frame ? frame->nLabels() : 1;
              renderer.setNCopies(copies > 0 ? copies : nLabels);
              renderer.setStartItem(startItem);
              renderer.setPrintOutlines(outlines);
              renderer.setPrintCropMarks(cropMarks);
              renderer.setPrintReverse(reverse);
              renderer.setIsCollated(collated);
              renderer.setAreGroupsContiguous(groupsContiguous);

              render_to_pdf_impl(renderer, filename);
          },
          py::arg("model"), py::arg("filename"),
          py::arg("copies") = 0, py::arg("start_item") = 0,
          py::arg("outlines") = false, py::arg("crop_marks") = false,
          py::arg("reverse") = false, py::arg("collated") = false,
          py::arg("groups_contiguous") = true,
          "Render a model to PDF with optional parameters");
}
