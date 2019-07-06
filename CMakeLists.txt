cmake_minimum_required(VERSION 3.11)

project(sofea)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "../cmake/")

# C++17 Support
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Local Paths
set(BOOST_ROOT "D:/Lib/boost-1.68.0")
set(SZIP_LIBRARY_DIR "D:/Lib/szip-2.1.1-vc141-md-x64/lib")
set(ZLIB_LIBRARY_DIR "D:/Lib/zlib-1.2.11-vc141-md-x64/lib")
set(CURL_LIBRARY_DIR "D:/Lib/curl-7.62.0-vc141-md-x64/lib")
set(HDF5_DIR "D:/Lib/hdf5-1.8.21-vc141-md-x64/cmake/hdf5")
set(NETCDF_INCLUDE_DIR "D:/Lib/netcdf-c-4.6.2/include")
set(NETCDF_LIBRARY "D:/Lib/netcdf-c-4.6.2-vc141-md-x64/lib")

# Options
option(SOFEA_DEBUG "Enable verbose logging" OFF)

#############################
# Qt
#############################

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5WinExtras REQUIRED)

set(QT5_LIBRARIES Qt5::Widgets Qt5::Svg Qt5::Network Qt5::WinExtras)

#############################
# Boost
#############################

set(BOOST_INCLUDEDIR ${BOOST_ROOT}/boost)
set(BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib)

#set(Boost_DEBUG ON)
set(BOOST_USE_STATIC_LIBS ON)
set(BOOST_USE_MULTITHREADED ON)
set(BOOST_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.68.0)

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${BOOST_LIBRARYDIR})

#############################
# netCDF Dependencies
#############################

set(SZIP_LIBRARY "libszip")
set(ZLIB_LIBRARY "zlib")
set(CURL_LIBRARY "libcurl_a")
link_directories(${SZIP_LIBRARY_DIR})
link_directories(${ZLIB_LIBRARY_DIR})
link_directories(${CURL_LIBRARY_DIR})

#############################
# HDF5
#############################

set(HDF5_USE_STATIC_LIBRARIES ON)
find_package(HDF5 COMPONENTS C HL REQUIRED)
set(HDF5_LIBRARIES ${HDF5_C_LIBRARY} ${HDF5_C_HL_LIBRARY})
include_directories(${HDF5_INCLUDE_DIR})

#############################
# netCDF
#############################

include_directories(${NETCDF_INCLUDE_DIR})
link_directories(${NETCDF_LIBRARY})
set(NETCDF_LIBRARIES "netcdf")

#############################
# Qwt
#############################

set(QWT_INCLUDE_DIR "../external/qwt/include")
set(QWT_LIBRARY_DIR "../external/qwt/lib")
set(QWT_LIBRARIES "qwt")
include_directories(${QWT_INCLUDE_DIR})
link_directories(${QWT_LIBRARY_DIR})

#############################
# QwtPolar
#############################

set(QWTPOLAR_INCLUDE_DIR "../external/qwtpolar/include")
set(QWTPOLAR_LIBRARY_DIR "../external/qwtpolar/lib")
set(QWTPOLAR_LIBRARIES "qwtpolar")
include_directories(${QWTPOLAR_INCLUDE_DIR})
link_directories(${QWTPOLAR_LIBRARY_DIR})

#############################
# cereal
#############################

set(CEREAL_INCLUDE_DIR "../external/cereal/include")
include_directories(${CEREAL_INCLUDE_DIR})

#############################
# csv
#############################

set(CSV_INCLUDE_DIR "../external/csv/include")
include_directories(${CSV_INCLUDE_DIR})

#############################
# fmtlib
#############################

set(FMT_INCLUDE_DIR "../external/fmt/include")
set(FMT_LIBRARY_DIR "../external/fmt/lib")
set(FMT_LIBRARIES "fmt")
include_directories(${FMT_INCLUDE_DIR})
link_directories(${FMT_LIBRARY_DIR})

#############################
# shapelib
#############################

set(SHAPELIB_INCLUDE_DIR "../external/shapelib/include")
set(SHAPELIB_LIBRARY_DIR "../external/shapelib/lib")
set(SHAPELIB_LIBRARIES "shapelib")
include_directories(${SHAPELIB_INCLUDE_DIR})
link_directories(${SHAPELIB_LIBRARY_DIR})

#############################
# geos
#############################

set(GEOS_INCLUDE_DIR "../external/geos/include")
set(GEOS_LIBRARY_DIR "../external/geos/lib")
set(GEOS_LIBRARIES "geos_c_i")
include_directories(${GEOS_INCLUDE_DIR})
link_directories(${GEOS_LIBRARY_DIR})

#############################
# expat
#############################

set(EXPAT_INCLUDE_DIR "../external/expat/include")
set(EXPAT_LIBRARY_DIR "../external/expat/lib")
set(EXPAT_LIBRARIES "expat")
include_directories(${EXPAT_INCLUDE_DIR})
link_directories(${EXPAT_LIBRARY_DIR})

#############################
# UDUNITS-2
#############################

set(UDUNITS2_INCLUDE_DIR "../external/udunits2/include")
set(UDUNITS2_LIBRARY_DIR "../external/udunits2/lib")
set(UDUNITS2_LIBRARIES "udunits2")
include_directories(${UDUNITS2_INCLUDE_DIR})
link_directories(${UDUNITS2_LIBRARY_DIR})

#############################
# Scintilla
#############################

set(SCINTILLA_ROOT "../external/scintilla")
set(SCINTILLA_LIBRARY_DIR "../external/scintilla/lib")
set(SCINTILLA_LIBRARIES "ScintillaEditBase")
include_directories(
    ${SCINTILLA_ROOT}/qt/ScintillaEditBase
    ${SCINTILLA_ROOT}/include
    ${SCINTILLA_ROOT}/src
    ${SCINTILLA_ROOT}/lexlib
)
link_directories(${SCINTILLA_LIBRARY_DIR})

#############################
# Target
#############################

set(TARGET ${CMAKE_PROJECT_NAME})

set(SOURCES
    ctkCollapsibleGroupBox.cpp
    ctkProxyStyle.cpp
    ctkRangeSlider.cpp
    main.cpp
    Analysis.cpp
    AnalysisWindow.cpp
    BufferZoneModel.cpp
    CurveTracker.cpp
    DateTimeDistributionDialog.cpp
    FluxProfileDialog.cpp
    FluxProfileModel.cpp
    FluxProfilePlot.cpp
    GenericDistribution.cpp
    GenericDistributionDialog.cpp
    GeometryOp.cpp
    GeosOp.cpp
    InputViewer.cpp
    InputWriter.cpp
    IPCServer.cpp
    ItemDelegate.cpp
    LogWidget.cpp
    MainWindow.cpp
    MetFileInfoDialog.cpp
    MetFileParser.cpp
    MonteCarloDateTimeEdit.cpp
    MonteCarloLineEdit.cpp
    PixmapUtilities.cpp
    ReceptorDialog.cpp
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
    StandardTableEditor.cpp
    StandardTableView.cpp
    UDUnitsLineEdit.cpp
    Utilities.cpp
    sofea.qrc
    sofea.rc
)

set(HEADERS
    ctkCollapsibleGroupBox.h
    ctkProxyStyle.h
    ctkRangeSlider.h
    Common.h
    Analysis.h
    AnalysisWindow.h
    BufferZone.h
    BufferZoneModel.h
    CurveTracker.h
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
    InputViewer.h
    InputWriter.h
    IPCMessage.h
    IPCServer.h
    ItemDelegate.h
    ListEditor.h
    LogWidget.h
    MainWindow.h
    MetFileInfoDialog.h
    MetFileParser.h
    MonteCarloDateTimeEdit.h
    MonteCarloLineEdit.h
    PixmapUtilities.h
    ReceptorDialog.h
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
    StandardTableEditor.h
    StandardTableView.h
    UDUnitsInterface.h
    UDUnitsLineEdit.h
    Utilities.h
)

qt5_wrap_ui(UI_GENERATED_HEADERS)
add_definitions(-DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE)

# These warnings are associated with Boost Accumulators
add_definitions(-D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
add_definitions(-D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING)

# Increase type limit for boost::variant
add_definitions(-DBOOST_MPL_CFG_NO_PREPROCESSED_HEADERS)
add_definitions(-DBOOST_MPL_LIMIT_LIST_SIZE=30)

add_definitions(-DCSV_IO_NO_THREAD)
add_definitions(-DSCINTILLA_QT)
add_definitions(-DSCI_LEXER)
add_definitions(-D_USE_MATH_DEFINES)
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if(SOFEA_DEBUG)
    add_definitions(-DSOFEA_DEBUG)
endif()

# Enable OpenMP
#find_package(OpenMP)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

set(EXTRA_LIBS "crypt32.lib" "wldap32.lib" "normaliz.lib" "wsock32.lib" "ws2_32.lib")

# Add WIN32 after ${TARGET} in add_executable to turn off console window
# ${HEADERS} in add_executable for Qt Creator support

#add_executable(${TARGET} WIN32 ${SOURCES})
add_executable(${TARGET} WIN32 ${SOURCES} ${HEADERS} ${UI_GENERATED_HEADERS})

target_link_libraries(${TARGET}
    ${EXTRA_LIBS}
    ${Boost_LOG_LIBRARY}
    ${Boost_DATE_TIME_LIBRARY}
    ${SZIP_LIBRARY}
    ${ZLIB_LIBRARY}
    ${CURL_LIBRARY}
    ${HDF5_LIBRARIES}
    ${NETCDF_LIBRARIES}
    ${QT5_LIBRARIES}
    ${QWT_LIBRARIES}
    ${QWTPOLAR_LIBRARIES}
    ${FMT_LIBRARIES}
    ${SHAPELIB_LIBRARIES}
    ${GEOS_LIBRARIES}
    ${EXPAT_LIBRARIES}
    ${UDUNITS2_LIBRARIES}
    ${SCINTILLA_LIBRARIES}
)
