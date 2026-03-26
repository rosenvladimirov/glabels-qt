# Install script for directory: /home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/glabels-qt/templates" TYPE FILE FILES
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/glabels-3.0.dtd"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/glabels-4.0.dtd"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/glabels-qt/templates" TYPE FILE FILES
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/ascom-iso-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/avery-iso-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/avery-other-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/avery-us-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/begalabel-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/brother-other-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/cablelabel-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/canon-other-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/databecker-iso-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/dataline-iso-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/decadry-iso-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/desmat-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/dymo-other-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/endisch-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/felga-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/geha-iso-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/hama-iso-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/herma-iso-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/hisago-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/igepa-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/jac-iso-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/maco-us-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/mayspies-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/meritline-us-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/microapp-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/misc-iso-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/misc-other-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/misc-us-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/online-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/pearl-iso-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/rayfilm-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/sheetlabels-us-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/uline-us-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/worldlabel-us-templates.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/zweckform-iso-templates.xml"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/glabels-qt/templates" TYPE FILE FILES
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/categories.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/paper-sizes.xml"
    "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/templates/vendors.xml"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/rosen/Проекти/glabels-qt/glabels-qt-fork/build-python/templates/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
