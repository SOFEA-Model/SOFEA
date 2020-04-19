# Copyright 2020 Dow, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 3.15)

project(sofea)

# High-Level Dependency Tree:
#
# SOFEA
# ├── boost                         [static]
# ├── qt5-base                      [dynamic]
# │   ├── sqlite3                   [static]
# │   └── openssl                   [dynamic]
# ├── qt5-winextras                 [dynamic]
# │   └── qt5-multimedia            [dynamic]
# │       └── qt5-declarative       [dynamic]
# │           ├── qt5-imageformats  [dynamic]
# │           │   ├── tiff          [dynamic]
# │           │   ├── libwebp       [dynamic]
# │           │   └── jasper        [dynamic]
# │           └── qt5-svg           [dynamic]
# ├── netcdf                        [dynamic]
# │   ├── hdf5                      [static]
# │   │   ├── zlib                  [static]
# │   │   ├── szip                  [static]
# │   │   └── msmpi                 [dynamic]
# │   └── curl                      [dynamic]
# ├── geos                          [dynamic]
# ├── qwt                           [static]
# ├── qwtpolar                      [static]
# ├── scintilla                     [static]
# ├── shapelib                      [static]
# ├── udunits2                      [static]
# │   └── expat                     [static]
# ├── curl                          [dynamic]
# │   └── zlib                      [static]
# ├── proj                          [static]
# │   ├── curl                      [dynamic]
# │   ├── tiff                      [static]
# │   └── sqlite3                   [static]
# └── gdal                          [dynamic]
#     ├── curl                      [dynamic]
#     ├── proj                      [static]
#     ├── expat                     [static]
#     ├── sqlite3                   [static]
#     └── zlib                      [static]
#
# qt5-imageformats must be dynamically linked to tiff, libwebp, jasper.
#
# The default vcpkg triplet is configured as follows:
#
# set(VCPKG_TARGET_ARCHITECTURE x64)
# set(VCPKG_CRT_LINKAGE dynamic)
# set(VCPKG_LIBRARY_LINKAGE static)
# if(PORT MATCHES "netcdf-c|qt5-|geos|curl|tiff|libwebp|jasper")
#   set(VCPKG_LIBRARY_LINKAGE dynamic)
# endif()
#

if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET} AND NOT DEFINED VCPKG_TARGET_TRIPLET)
  set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
endif()

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

message("CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")
message("VCPKG_TARGET_TRIPLET: ${VCPKG_TARGET_TRIPLET}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

# Enable LTO (Experimental)
# /GL (Whole Program Optimization) in MSVC
#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

# Debugging
#set(VLD_INCLUDE_DIR "C:/Program Files (x86)/Visual Leak Detector/include/")
#set(VLD_LIBRARY_DIR "C:/Program Files (x86)/Visual Leak Detector/lib/Win64")
#set(VLD_LIBRARY "vld.lib")
#include_directories(${VLD_INCLUDE_DIR})
#link_directories(${VLD_LIBRARY_DIR})

# Ribbon UICC
set(SDKKEY "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\v7.1")
get_filename_component(SDKDIR "[${SDKKEY};InstallationFolder]" DIRECTORY)
set(UICC "${SDKDIR}\\v7.1\\Bin\\uicc.exe")

option(SOFEA_DEBUG "Enable verbose logging" OFF)

#############################
# External Libraries
#############################

#find_package(OpenMP)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

set(Boost_DEBUG OFF)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_ALL_DYN_LINK OFF)

find_package(Boost REQUIRED COMPONENTS THREAD LOG DATE_TIME)
find_package(Qt5Core CONFIG REQUIRED)
find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5Network CONFIG REQUIRED)
find_package(Qt5WinExtras CONFIG REQUIRED)
find_package(Qt5Sql CONFIG REQUIRED)
find_package(ZLIB REQUIRED QUIET)
find_package(EXPAT REQUIRED QUIET)
find_package(CURL REQUIRED QUIET)
find_package(date CONFIG REQUIRED QUIET)
find_package(fmt CONFIG REQUIRED QUIET)
find_package(cereal CONFIG REQUIRED QUIET)
find_package(netCDF CONFIG REQUIRED QUIET)

### Custom Builds
# - geos
# - proj
# - qwt
# - qwtpolar
# - shapelib
# - scintilla
#   - patched qt\ScintillaEditBase\ScintillaEditBase.h to remove DLL exports
#   - updated ScintillaEditBase.pro with CONFIG += static
# - udunits2
#   - changed DEFAULT_UDUNITS2_XML_PATH
#   - add #ifndef _USE_MATH_DEFINES to udunits2.h

find_package(geos PATHS "../external/geos-3.8.0dev/lib/cmake/GEOS" NO_DEFAULT_PATH CONFIG REQUIRED)

find_package(PROJ PATHS "../external/proj-7.0.0/lib/cmake/proj" NO_DEFAULT_PATH CONFIG REQUIRED)

set(GDAL_INCLUDE_DIR "../external/gdal-3.1.0dev/include")
set(GDAL_LIBRARY_DIR "../external/gdal-3.1.0dev/lib")
set(GDAL_LIBRARIES "gdal_i")

set(QWT_INCLUDE_DIR "../external/qwt-6.1.4/include")
set(QWT_LIBRARY_DIR "../external/qwt-6.1.4/lib")
set(QWT_LIBRARIES "qwt")

set(QWTPOLAR_INCLUDE_DIR "../external/qwtpolar-1.1.1/include")
set(QWTPOLAR_LIBRARY_DIR "../external/qwtpolar-1.1.1/lib")
set(QWTPOLAR_LIBRARIES "qwtpolar")

set(QTPROPERTYBROWSER_INCLUDE_DIR "../external/qtpropertybrowser/src")
set(QTPROPERTYBROWSER_LIBRARY_DIR "../external/qtpropertybrowser/lib")
set(QTPROPERTYBROWSER_LIBRARIES "QtPropertyBrowser")

set(SHAPELIB_INCLUDE_DIR "../external/shapelib/include")
set(SHAPELIB_LIBRARY_DIR "../external/shapelib/lib")
set(SHAPELIB_LIBRARIES "shapelib")

set(UDUNITS2_INCLUDE_DIR "../external/udunits-2.2.27.12/include")
set(UDUNITS2_LIBRARY_DIR "../external/udunits-2.2.27.12/lib")
set(UDUNITS2_LIBRARIES "udunits2")

set(SCINTILLA_ROOT "../external/scintilla-4.3.2")
set(SCINTILLA_LIBRARY_DIR "../external/scintilla-4.3.2/bin")
set(SCINTILLA_LIBRARIES "ScintillaEditBase")

set(CSV_INCLUDE_DIR "../external/fast-cpp-csv-parser/include")

### Internal Libraries

set(NCPP_INCLUDE_DIR "../ncpp/include")

message("-- Qt version: ${Qt5Core_VERSION}")
message("-- HDF5 version: ${HDF5_VERSION}")
message("-- netCDF version: ${netCDF_VERSION}")
message("-- curl version: ${CURL_VERSION_STRING}")
message("-- Proj version: ${PROJ_VERSION}")
message("-- GEOS version: ${geos_VERSION}")

#############################
# Sources
#############################

set(SOURCES
    main.cpp
    AnalysisWindow.cpp
    AppStyle.cpp
    BufferZoneEditor.cpp
    DateTimeDistributionDialog.cpp
    FluxProfileDialog.cpp
    FluxProfilePlot.cpp
    GenericDistributionDialog.cpp
    GeometryOp.cpp
    GeosOp.cpp
    InputViewer.cpp
    IPCServer.cpp
    LogWidget.cpp
    MagneticDeclinationDialog.cpp
    MainWindow.cpp
    MetFileInfoDialog.cpp
    MetFileParser.cpp
    MeteorologyEditor.cpp
    MeteorologyFileDialog.cpp
    MeteorologyStationData.cpp
    MeteorologyTableView.cpp
    MonteCarloDateTimeEdit.cpp
    MonteCarloLineEdit.cpp
    PixmapUtilities.cpp
    PlotCurveTracker.cpp
    PlotItemTitleTracker.cpp
    ProjectOptionsDialog.cpp
    ProjectTreeView.cpp
    ProjectionEditor.cpp
    ReceptorDialog.cpp
    ReceptorEditor.cpp
    ReceptorTreeView.cpp
    RunModelDialog.cpp
    RunstreamParser.cpp
    SamplingDistributionEditor.cpp
    ScenarioPages.cpp
    ScenarioProperties.cpp
    SettingsDialog.cpp
    ShapefileAttributeDialog.cpp
    ShapefileParser.cpp
    SourceEditor.cpp
    SourceGroupPages.cpp
    SourceGroupProperties.cpp
    SourceTable.cpp
    StandardPlot.cpp
    UDUnitsInterface.cpp
    UDUnitsLineEdit.cpp
    analysis/Analysis.cpp
    core/GenericDistribution.cpp
    core/InputWriter.cpp
    core/Meteorology.cpp
    core/Projection.cpp
    core/Receptor.cpp
    core/Scenario.cpp
    core/Source.cpp
    core/SourceGroup.cpp
    core/TerrainProcessor.cpp
    core/Validation.cpp
    ctk/ctkCollapsibleGroupBox.cpp
    ctk/ctkMenuButton.cpp
    ctk/ctkProxyStyle.cpp
    ctk/ctkRangeSlider.cpp
    delegates/ColorIntervalDelegate.cpp
    delegates/ColorPickerDelegate.cpp
    delegates/ComboBoxDelegate.cpp
    delegates/DateTimeEditDelegate.cpp
    delegates/DoubleItemDelegate.cpp
    delegates/DoubleSpinBoxDelegate.cpp
    delegates/FilePickerDelegate.cpp
    delegates/GradientItemDelegate.cpp
    delegates/PointDelegate.cpp
    delegates/ProgressBarDelegate.cpp
    delegates/SpinBoxDelegate.cpp
    delegates/VertexEditorDelegate.cpp
    models/BufferZoneModel.cpp
    models/FluxProfileModel.cpp
    models/LogFilterProxyModel.cpp
    models/MeteorologyModel.cpp
    models/ProjectModel.cpp
    models/ReceptorModel.cpp
    models/SamplingProxyModel.cpp
    models/SourceModel.cpp
    qtcurl/CurlEasy.cpp
    qtcurl/CurlMulti.cpp
    #ribbon/CRibbon.cpp
    #ribbon/RibbonWindow.cpp
    widgets/BackgroundFrame.cpp
    widgets/ButtonLineEdit.cpp
    widgets/DoubleLineEdit.cpp
    widgets/ExtendedComboBox.cpp
    widgets/FilterEditor.cpp
    widgets/FilterHeaderView.cpp
    widgets/FilterProxyModel.cpp
    widgets/GridLayout.cpp
    widgets/GridLineFrame.cpp
    widgets/ListEditor.cpp
    widgets/MenuLineEdit.cpp
    widgets/NavButton.cpp
    widgets/PathEdit.cpp
    widgets/PlainTextEdit.cpp
    widgets/ReadOnlyLineEdit.cpp
    widgets/ReadOnlyTextEdit.cpp
    widgets/StandardTableEditor.cpp
    widgets/StandardTableView.cpp
    widgets/StatusLabel.cpp
    widgets/VertexEditor.cpp
    sofea.qrc
    sofea.rc
)

#############################
# Headers
#############################

set(HEADERS
    AnalysisWindow.h
    AppStyle.h
    BufferZoneEditor.h
    FluxProfileDialog.h
    FluxProfilePlot.h
    DateTimeDistributionDialog.h
    GenericDistributionDialog.h
    GeometryOp.h
    GeosOp.h
    InputViewer.h
    IPCMessage.h
    IPCServer.h
    LogWidget.h
    MagneticDeclinationDialog.h
    MainWindow.h
    MeteorologyEditor.h
    MeteorologyFileDialog.h
    MeteorologyStationData.h
    MeteorologyTableView.h
    MetFileInfoDialog.h
    MetFileParser.h
    MonteCarloDateTimeEdit.h
    MonteCarloLineEdit.h
    PixmapUtilities.h
    PlotCurveTracker.h
    PlotItemTitleTracker.h
    ProjectOptionsDialog.h
    ProjectTreeView.h
    ProjectionEditor.h
    ReceptorDelegate.h
    ReceptorDialog.h
    ReceptorEditor.h
    ReceptorTreeView.h
    ReceptorVisitor.h
    RunModelDialog.h
    Runstream.h
    RunstreamParser.h
    SamplingDistribution.h
    SamplingDistributionEditor.h
    ScenarioPages.h
    ScenarioProperties.h
    SettingsDialog.h
    ShapefileAttributeDialog.h
    ShapefileAttributeInfo.h
    ShapefileParser.h
    SourceEditor.h
    SourceGroupPages.h
    SourceGroupProperties.h
    SourceTable.h
    StandardPlot.h
    UDUnitsInterface.h
    UDUnitsLineEdit.h
    analysis/Analysis.h
    analysis/AnalysisOptions.h
    core/BufferZone.h
    core/Common.h
    core/DateTimeConversion.h
    core/DateTimeDistribution.h
    core/FluxProfile.h
    core/GenericDistribution.h
    core/InputFormat.h
    core/InputWriter.h
    core/Meteorology.h
    core/Project.h
    core/Projection.h
    core/Receptor.h
    core/Scenario.h
    core/Serialization.h
    core/Source.h
    core/SourceGroup.h
    core/TerrainProcessor.h
    core/Validation.h
    ctk/ctkCollapsibleGroupBox.h
    ctk/ctkMenuButton.h
    ctk/ctkProxyStyle.h
    ctk/ctkRangeSlider.h
    delegates/ColorIntervalDelegate.h
    delegates/ColorPickerDelegate.h
    delegates/ComboBoxDelegate.h
    delegates/DateTimeEditDelegate.h
    delegates/DoubleItemDelegate.h
    delegates/DoubleSpinBoxDelegate.h
    delegates/FilePickerDelegate.h
    delegates/GradientItemDelegate.h
    delegates/PointDelegate.h
    delegates/ProgressBarDelegate.h
    delegates/SpinBoxDelegate.h
    delegates/VertexEditorDelegate.h
    models/BufferZoneModel.h
    models/FluxProfileModel.h
    models/LogFilterProxyModel.h
    models/MeteorologyModel.h
    models/ProjectModel.h
    models/ReceptorModel.h
    models/SamplingProxyModel.h
    models/SourceModel.h
    qtcurl/CurlEasy.h
    qtcurl/CurlMulti.h
    #ribbon/CRibbon.h
    #ribbon/RibbonWindow.h
    widgets/BackgroundFrame.h
    widgets/ButtonLineEdit.h
    widgets/DoubleLineEdit.h
    widgets/ExtendedComboBox.h
    widgets/FilterEditor.h
    widgets/FilterHeaderView.h
    widgets/GridLayout.h
    widgets/GridLineFrame.h
    widgets/ListEditor.h
    widgets/MenuLineEdit.h
    widgets/NavButton.h
    widgets/PathEdit.h
    widgets/PlainTextEdit.h
    widgets/ReadOnlyLineEdit.h
    widgets/ReadOnlyTextEdit.h
    widgets/StandardTableEditor.h
    widgets/StandardTableView.h
    widgets/StatusLabel.h
    widgets/VertexEditor.h
)

#############################
# Targets
#############################

#set(RIBBON_XML "${CMAKE_CURRENT_SOURCE_DIR}/RibbonDefinition.xml")
#set(RIBBON_H   "${CMAKE_CURRENT_SOURCE_DIR}/RibbonDefinition.h")
#set(RIBBON_RC  "${CMAKE_CURRENT_SOURCE_DIR}/RibbonDefinition.rc")
#set(RIBBON_BML "${CMAKE_CURRENT_BINARY_DIR}/RibbonDefinition.bml")
#
#file(TO_NATIVE_PATH "${RIBBON_XML}" RIBBON_XML_PATH)
#file(TO_NATIVE_PATH "${RIBBON_H}"   RIBBON_H_PATH)
#file(TO_NATIVE_PATH "${RIBBON_BML}" RIBBON_BML_PATH)
#file(TO_NATIVE_PATH "${RIBBON_RC}"  RIBBON_RC_PATH)
#
#add_custom_command(
#    OUTPUT ${RIBBON_BML} ${RIBBON_H} ${RIBBON_RC}
#    COMMAND ${UICC} ${RIBBON_XML_PATH} ${RIBBON_BML_PATH} /header:${RIBBON_H_PATH} /res:${RIBBON_RC_PATH}
#    DEPENDS ${RIBBON_XML}
#    COMMENT "Running UICC"
#)
#list(APPEND HEADERS ${RIBBON_H})
#add_custom_target(xml ALL SOURCES ${RIBBON_XML})

qt5_wrap_ui(UI_GENERATED_HEADERS)

add_executable(main WIN32 ${SOURCES} ${HEADERS} ${UI_GENERATED_HEADERS})
#set_source_files_properties(${RIBBON_H} PROPERTIES GENERATED 1)
set_target_properties(main PROPERTIES OUTPUT_NAME ${CMAKE_PROJECT_NAME})

#############################
# Definitions
#############################

# Win32 definitions
target_compile_definitions(main PRIVATE -DUNICODE -D_UNICODE)

# Boost definitions
target_compile_definitions(main PRIVATE -DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE)

# Increase type limit for boost::variant
target_compile_definitions(main PRIVATE -DBOOST_MPL_CFG_NO_PREPROCESSED_HEADERS)
target_compile_definitions(main PRIVATE -DBOOST_MPL_LIMIT_LIST_SIZE=30)

# MSVC definitions
if(MSVC)
    target_compile_definitions(main PRIVATE -D_USE_MATH_DEFINES)
    target_compile_definitions(main PRIVATE -D_CRT_SECURE_NO_WARNINGS)

    # Suppress warnings associated with Boost Accumulators
    target_compile_definitions(main PRIVATE -D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
    target_compile_definitions(main PRIVATE -D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING)

    # Suppress warnings associated with Boost uBLAS
    target_compile_definitions(main PRIVATE -D_SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING)
endif()

# Scintilla definitions
target_compile_definitions(main PRIVATE -DSCINTILLA_QT)
target_compile_definitions(main PRIVATE -DSCI_LEXER)

# Internal definitions
if(SOFEA_DEBUG)
    target_compile_definitions(main PRIVATE -DSOFEA_DEBUG)
endif()

target_include_directories(main PRIVATE
    ${CURL_INCLUDE_DIRS}
    ${GDAL_INCLUDE_DIR}
    ${QWT_INCLUDE_DIR}
    ${QWTPOLAR_INCLUDE_DIR}
    ${QTPROPERTYBROWSER_INCLUDE_DIR}
    ${CSV_INCLUDE_DIR}
    ${NCPP_INCLUDE_DIR}
    ${SHAPELIB_INCLUDE_DIR}
    ${SCINTILLA_ROOT}/qt/ScintillaEditBase
    ${SCINTILLA_ROOT}/include
    ${SCINTILLA_ROOT}/src
    ${SCINTILLA_ROOT}/lexlib
    ${UDUNITS2_INCLUDE_DIR}
)

target_link_directories(main PRIVATE
    ${GDAL_LIBRARY_DIR}
    ${QWT_LIBRARY_DIR}
    ${QWTPOLAR_LIBRARY_DIR}
    ${QTPROPERTYBROWSER_LIBRARY_DIR}
    ${SHAPELIB_LIBRARY_DIR}
    ${SCINTILLA_LIBRARY_DIR}
    ${UDUNITS2_LIBRARY_DIR}
    ${netCDF_LIB_DIR}
)

# Qt Libraries
target_link_libraries(main PRIVATE Qt5::Core Qt5::WinMain Qt5::Widgets Qt5::Network Qt5::Sql Qt5::WinExtras)

# Win32 Libraries
#target_link_libraries(main PRIVATE user32 gdi32 ole32 shlwapi propsys)

target_link_libraries(main PRIVATE ${Boost_LIBRARIES})
target_link_libraries(main PRIVATE ${CURL_LIBRARIES})
target_link_libraries(main PRIVATE netcdf)
target_link_libraries(main PRIVATE GEOS::geos_c)
target_link_libraries(main PRIVATE PROJ::proj)
target_link_libraries(main PRIVATE EXPAT::EXPAT)
target_link_libraries(main PRIVATE date::date date::tz)
target_link_libraries(main PRIVATE fmt::fmt-header-only)
target_link_libraries(main PRIVATE cereal)
target_link_libraries(main PRIVATE ${GDAL_LIBRARIES})
target_link_libraries(main PRIVATE ${QWT_LIBRARIES})
target_link_libraries(main PRIVATE ${QWTPOLAR_LIBRARIES})
target_link_libraries(main PRIVATE ${QTPROPERTYBROWSER_LIBRARIES})
target_link_libraries(main PRIVATE ${SHAPELIB_LIBRARIES})
target_link_libraries(main PRIVATE ${UDUNITS2_LIBRARIES})
target_link_libraries(main PRIVATE ${SCINTILLA_LIBRARIES})
