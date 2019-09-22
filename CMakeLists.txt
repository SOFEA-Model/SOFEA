cmake_minimum_required(VERSION 3.15)

project(sofea)

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

### Enable LTO (Experimental)
# /GL (Whole Program Optimization) in MSVC
#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

### Debugging
#set(VLD_INCLUDE_DIR "C:/Program Files (x86)/Visual Leak Detector/include/")
#set(VLD_LIBRARY_DIR "C:/Program Files (x86)/Visual Leak Detector/lib/Win64")
#set(VLD_LIBRARY "vld.lib")
#include_directories(${VLD_INCLUDE_DIR})
#link_directories(${VLD_LIBRARY_DIR})

option(SOFEA_DEBUG "Enable verbose logging" OFF)

#############################
# Find External Libraries
#############################

set(Boost_DEBUG OFF)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_ALL_DYN_LINK OFF)

### Dependencies:
### hdf5: zlib
### netcdf: hdf5
### curl: zlib
### proj: sqlite3
### udunits2: expat

### Vcpkg

find_package(Boost REQUIRED COMPONENTS THREAD LOG DATE_TIME)
find_package(Qt5Core CONFIG REQUIRED)
find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5Network CONFIG REQUIRED)
find_package(Qt5WinExtras CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(EXPAT REQUIRED)
find_package(CURL REQUIRED)
find_package(PROJ4 REQUIRED)
find_package(geos CONFIG REQUIRED)
find_package(date CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(cereal CONFIG REQUIRED)

### Custom Builds

#set(GEOS_DIR "E:/Lib/geos-3.8.0dev-vc141-md-x64-shared/lib/cmake/GEOS")
#set(GDAL_DIR "E:/Lib/gdal-dev-vc141-md-x64-shared")
#set(EXPAT_DIR "E:/Lib/expat-2.2.8-vc141-md-x64-static/lib/cmake/expat-2.2.8")
#set(PROJ4_DIR "E:/Lib/proj-6.2.0-vc141-md-x64-shared/lib/cmake/proj4")

set(NETCDF_DIR "E:/Lib/netcdf-c-4.7.1-vc141-md-x64-shared/lib/cmake/netCDF")
find_package(NETCDF CONFIG REQUIRED)

set(QWT_INCLUDE_DIR "../external/qwt-6.1.4/include")
set(QWT_LIBRARY_DIR "../external/qwt-6.1.4/lib")
set(QWT_LIBRARIES "qwt")

set(QWTPOLAR_INCLUDE_DIR "../external/qwtpolar-1.1.1/include")
set(QWTPOLAR_LIBRARY_DIR "../external/qwtpolar-1.1.1/lib")
set(QWTPOLAR_LIBRARIES "qwtpolar")

set(SHAPELIB_INCLUDE_DIR "../external/shapelib/include")
set(SHAPELIB_LIBRARY_DIR "../external/shapelib/lib")
set(SHAPELIB_LIBRARIES "shapelib")

set(UDUNITS2_INCLUDE_DIR "../external/udunits-2.2.27.12/include")
set(UDUNITS2_LIBRARY_DIR "../external/udunits-2.2.27.12/lib")
set(UDUNITS2_LIBRARIES "udunits2")

set(SCINTILLA_ROOT "../external/scintilla-4.2.0")
set(SCINTILLA_LIBRARY_DIR "../external/scintilla-4.2.0/bin")
set(SCINTILLA_LIBRARIES "ScintillaEditBase")

set(CSV_INCLUDE_DIR "../external/fast-cpp-csv-parser/include")

### Internal Libraries

set(NCPP_INCLUDE_DIR "../ncpp/include")

### OpenMP
#find_package(OpenMP)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

message("-- Qt Version: ${Qt5Core_VERSION}")
message("-- NetCDF Version: ${NetCDFVersion}")
message("-- Proj Version: ${PROJ4_VERSION}")
message("-- GEOS Version: ${geos_VERSION}")

#############################
# Sources
#############################

set(SOURCES
    main.cpp
    AnalysisWindow.cpp
    AppStyle.cpp
    BufferZoneModel.cpp
    DateTimeDistributionDialog.cpp
    FluxProfileDialog.cpp
    FluxProfileModel.cpp
    FluxProfilePlot.cpp
    GenericDistribution.cpp
    GenericDistributionDialog.cpp
    GeometryOp.cpp
    GeosOp.cpp
    InputViewer.cpp
    #InputWriter.cpp
    IPCServer.cpp
    LogWidget.cpp
    MagneticDeclinationDialog.cpp
    MainWindow.cpp
    MetFileInfoDialog.cpp
    MetFileParser.cpp
    MonteCarloDateTimeEdit.cpp
    MonteCarloLineEdit.cpp
    PixmapUtilities.cpp
    PlotCurveTracker.cpp
    PlotItemTitleTracker.cpp
    #ProjectModel.cpp
    #ProjectOptionsDialog.h
    #ProjectTreeView.h
    ProjectValidation.cpp
    Projection.cpp
    ProjectionEditor.cpp
    Receptor.cpp
    ReceptorDialog.cpp
    ReceptorEditor.cpp
    ReceptorModel.cpp
    ReceptorTreeView.cpp
    RunModelDialog.cpp
    RunstreamParser.cpp
    SamplingDistributionEditor.cpp
    SamplingProxyModel.cpp
    Scenario.cpp
    ScenarioPages.cpp
    ScenarioProperties.cpp
    SettingsDialog.cpp
    ShapefileAttributeDialog.cpp
    ShapefileParser.cpp
    Source.cpp
    SourceEditor.cpp
    SourceGroup.cpp
    SourceGroupPages.cpp
    SourceGroupProperties.cpp
    SourceModel.cpp
    SourceTable.cpp
    StandardPlot.cpp
    UDUnitsInterface.cpp
    UDUnitsLineEdit.cpp
    ctk/ctkCollapsibleGroupBox.cpp
    ctk/ctkProxyStyle.cpp
    ctk/ctkRangeSlider.cpp
    analysis/Analysis.cpp
    delegate/ColorIntervalDelegate.cpp
    delegate/ColorPickerDelegate.cpp
    delegate/ComboBoxDelegate.cpp
    delegate/DateTimeEditDelegate.cpp
    delegate/DoubleItemDelegate.cpp
    delegate/DoubleSpinBoxDelegate.cpp
    delegate/GradientItemDelegate.cpp
    delegate/PointDelegate.cpp
    delegate/ProgressBarDelegate.cpp
    delegate/SpinBoxDelegate.cpp
    widgets/BackgroundFrame.cpp
    widgets/DoubleLineEdit.cpp
    widgets/ExtendedComboBox.cpp
    widgets/FilterHeaderView.cpp
    widgets/GridLayout.cpp
    widgets/GridLineFrame.cpp
    widgets/ListEditor.cpp
    widgets/NavButton.cpp
    widgets/PlainTextEdit.cpp
    widgets/ReadOnlyLineEdit.cpp
    widgets/ReadOnlyTextEdit.cpp
    widgets/StandardTableEditor.cpp
    widgets/StandardTableView.cpp
    widgets/StatusLabel.cpp
    sofea.qrc
    sofea.rc
)

#############################
# Headers
#############################

set(HEADERS
    Common.h
    AnalysisWindow.h
    AppStyle.h
    BufferZone.h
    BufferZoneModel.h
    FluxProfile.h
    FluxProfileDialog.h
    FluxProfileModel.h
    FluxProfilePlot.h
    DateTimeDistribution.h
    DateTimeDistributionDialog.h
    GenericDistribution.h
    GenericDistributionDialog.h
    GeometryOp.h
    GeosOp.h
    InputFormat.h
    InputViewer.h
    #InputWriter.h
    IPCMessage.h
    IPCServer.h
    LogWidget.h
    MagneticDeclinationDialog.h
    MainWindow.h
    MetFileInfoDialog.h
    MetFileParser.h
    MonteCarloDateTimeEdit.h
    MonteCarloLineEdit.h
    PixmapUtilities.h
    PlotCurveTracker.h
    PlotItemTitleTracker.h
    Project.h
    #ProjectModel.h
    #ProjectOptionsDialog.h
    #ProjectTreeView.h
    ProjectValidation.h
    Projection.h
    ProjectionEditor.h
    Receptor.h
    ReceptorDelegate.h
    ReceptorDialog.h
    ReceptorEditor.h
    ReceptorModel.h
    ReceptorTreeView.h
    ReceptorVisitor.h
    RunModelDialog.h
    Runstream.h
    RunstreamParser.h
    SamplingDistribution.h
    SamplingDistributionEditor.h
    SamplingProxyModel.h
    Scenario.h
    ScenarioPages.h
    ScenarioProperties.h
    Serialization.h
    SettingsDialog.h
    ShapefileAttributeDialog.h
    ShapefileAttributeInfo.h
    ShapefileParser.h
    Source.h
    SourceEditor.h
    SourceGroup.h
    SourceGroupPages.h
    SourceGroupProperties.h
    SourceModel.h
    SourceTable.h
    StandardPlot.h
    UDUnitsInterface.h
    UDUnitsLineEdit.h
    ctk/ctkCollapsibleGroupBox.h
    ctk/ctkProxyStyle.h
    ctk/ctkRangeSlider.h
    analysis/Analysis.h
    analysis/AnalysisOptions.h
    delegate/ColorIntervalDelegate.h
    delegate/ColorPickerDelegate.h
    delegate/ComboBoxDelegate.h
    delegate/DateTimeEditDelegate.h
    delegate/DoubleItemDelegate.h
    delegate/DoubleSpinBoxDelegate.h
    delegate/GradientItemDelegate.h
    delegate/PointDelegate.h
    delegate/ProgressBarDelegate.h
    delegate/SpinBoxDelegate.h
    widgets/BackgroundFrame.h
    widgets/DoubleLineEdit.h
    widgets/ExtendedComboBox.h
    widgets/FilterHeaderView.h
    widgets/GridLayout.h
    widgets/GridLineFrame.h
    widgets/ListEditor.h
    widgets/NavButton.h
    widgets/PlainTextEdit.h
    widgets/ReadOnlyLineEdit.h
    widgets/ReadOnlyTextEdit.h
    widgets/StandardTableEditor.h
    widgets/StandardTableView.h
    widgets/StatusLabel.h
)

#############################
# Target
#############################

qt5_wrap_ui(UI_GENERATED_HEADERS)

# ${HEADERS} in add_executable for Qt Creator support
add_executable(main WIN32 ${SOURCES} ${HEADERS} ${UI_GENERATED_HEADERS})
set_target_properties(main PROPERTIES OUTPUT_NAME ${CMAKE_PROJECT_NAME})

#############################
# Definitions
#############################

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

# SOFEA definitions
if(SOFEA_DEBUG)
    target_compile_definitions(main PRIVATE -DSOFEA_DEBUG)
endif()

target_include_directories(main PRIVATE
    ${CURL_INCLUDE_DIRS}
    ${NETCDF_INCLUDE_DIR}
    ${QWT_INCLUDE_DIR}
    ${QWTPOLAR_INCLUDE_DIR}
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
    ${NETCDF_LIBRARY}
    ${QWT_LIBRARY_DIR}
    ${QWTPOLAR_LIBRARY_DIR}
    ${SHAPELIB_LIBRARY_DIR}
    ${SCINTILLA_LIBRARY_DIR}
    ${UDUNITS2_LIBRARY_DIR}
)

target_link_libraries(main PUBLIC Qt5::Core Qt5::WinMain Qt5::Widgets Qt5::Network Qt5::WinExtras)
target_link_libraries(main PRIVATE ${Boost_LIBRARIES})
target_link_libraries(main PRIVATE ${CURL_LIBRARIES})
target_link_libraries(main PUBLIC netcdf)
target_link_libraries(main PUBLIC GEOS::geos_c)
target_link_libraries(main PRIVATE PROJ4::proj)
target_link_libraries(main PRIVATE EXPAT::EXPAT)
target_link_libraries(main PRIVATE date::date date::tz)
target_link_libraries(main PRIVATE fmt::fmt-header-only)
target_link_libraries(main PRIVATE cereal)

target_link_libraries(main PRIVATE ${QWT_LIBRARIES})
target_link_libraries(main PRIVATE ${QWTPOLAR_LIBRARIES})
target_link_libraries(main PRIVATE ${SHAPELIB_LIBRARIES})
target_link_libraries(main PRIVATE ${UDUNITS2_LIBRARIES})
target_link_libraries(main PRIVATE ${SCINTILLA_LIBRARIES})
