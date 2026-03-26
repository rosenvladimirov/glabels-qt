find_package (Python3 REQUIRED COMPONENTS Interpreter Development.Module)
find_package (pybind11 CONFIG REQUIRED)

pybind11_add_module (glabels_ext
  glabels_ext.cpp
  bind_types.cpp
  bind_template.cpp
  bind_model.cpp
  bind_io.cpp
  bind_renderer.cpp
  bind_merge.cpp
  bind_barcode.cpp
)

target_include_directories (glabels_ext PRIVATE
  ${CMAKE_SOURCE_DIR}
)

target_link_libraries (glabels_ext PRIVATE
  Model
  Qt6::Core
  Qt6::Gui
  Qt6::PrintSupport
  Qt6::Xml
  Qt6::Svg
)

install (TARGETS glabels_ext DESTINATION glabels)
