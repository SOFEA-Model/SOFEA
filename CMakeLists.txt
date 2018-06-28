cmake_minimum_required(VERSION 3.11)

project(sofea)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "../cmake/")

# C++17 Support
#if(MSVC)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
#endif(MSVC)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Local Paths

set(QWT_ROOT "C:/Qwt-6.1.3")
set(QWTPOLAR_ROOT "C:/QwtPolar-1.2.0-svn")
set(BOOST_ROOT "C:/boost_1_67_0")
set(HDF5_ROOT "C:/Program Files/HDF_Group/HDF5/1.8.20")
set(NETCDF_INCLUDE_DIR "C:/Program Files/netCDF/include")
set(NETCDF_LIBRARY "C:/Program Files/netCDF/lib")
set(FMT_LIBRARY_DIR "../lib")

#############################
# Qt
#############################

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5Network REQUIRED)

set(QT5_LIBRARIES Qt5::Widgets Qt5::Svg Qt5::Network)

#############################
# Qwt
#############################

set(QWT_INCLUDE_DIR ${QWT_ROOT}/include)
set(QWT_LIBRARY_DIR ${QWT_ROOT}/lib)
set(QWT_LIBRARIES "qwt")

include_directories(${QWT_INCLUDE_DIR})
link_directories(${QWT_LIBRARY_DIR})

#############################
# QwtPolar
#############################

set(QWTPOLAR_INCLUDE_DIR ${QWTPOLAR_ROOT}/include)
set(QWTPOLAR_LIBRARY_DIR ${QWTPOLAR_ROOT}/lib)
set(QWTPOLAR_LIBRARIES "qwtpolar")

include_directories(${QWTPOLAR_INCLUDE_DIR})
link_directories(${QWTPOLAR_LIBRARY_DIR})

#############################
# Boost
#############################

set(BOOST_INCLUDEDIR ${BOOST_ROOT}/boost)
set(BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib)

#set(Boost_DEBUG ON)

set(BOOST_USE_STATIC_LIBS ON)
set(BOOST_USE_MULTITHREADED ON)
set(BOOST_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.67.0)

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

#############################
# HDF5
#############################

find_package(HDF5 COMPONENTS C HL NO_MODULE REQUIRED static)

include_directories(${HDF5_INCLUDE_DIRS})
link_directories(${HDF5_C_LIBRARY_DIRS})

#############################
# netCDF
#############################

set(NETCDF_C "YES")
find_package(NetCDF REQUIRED)

set(NETCDF_LIBRARIES "netcdf")
include_directories(${NETCDF_INCLUDE_DIR})
link_directories(${NETCDF_LIBRARY})

#############################
# fmtlib
#############################

set(FMT_LIBRARIES "fmt")

link_directories(${FMT_LIBRARY_DIR})

#############################
# Target
#############################

set(TARGET ${CMAKE_PROJECT_NAME})

set(SOURCES
    ctkRangeSlider.cpp
    main.cpp
    Analysis.cpp
    AnalysisWindow.cpp
    CurveTracker.cpp
    GenericDistribution.cpp
    GenericDistributionDialog.cpp
    GeometryOp.cpp
    InputViewer.cpp
    IPCServer.cpp
    ItemDelegate.cpp
    LogWidget.cpp
    MainWindow.cpp
    MetFileInfoDialog.cpp
    MetFileParser.cpp
    MonteCarloDateTimeEdit.cpp
    MonteCarloLineEdit.cpp
    ReceptorDialog.cpp
    RunModelDialog.cpp
    RunstreamParser.cpp
    Scenario.cpp
    ScenarioPages.cpp
    ScenarioProperties.cpp
    SettingsDialog.cpp
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
    Utilities.cpp
    sofea.qrc
    sofea.rc
)

set(HEADERS
    ctkRangeSlider.h
    Common.h
    Analysis.h
    AnalysisWindow.h
    CurveTracker.h
    FluxScaling.h
    GenericDistribution.h
    GenericDistributionDialog.h
    GeometryOp.h
    InputViewer.h
    IPCServer.h
    ItemDelegate.h
    ListEditor.h
    LogWidget.h
    MainWindow.h
    MetFileInfoDialog.h
    MetFileParser.h
    MonteCarloDateTimeEdit.h
    MonteCarloLineEdit.h
    ReceptorDialog.h
    RunModelDialog.h
    Runstream.h
    RunstreamParser.h
    Scenario.h
    ScenarioPages.h
    ScenarioProperties.h
    Serialization.h
    SettingsDialog.h
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
    Utilities.h
)

# Add WIN32 after ${TARGET} in add_executable to turn off console window
# ${HEADERS} in add_executable for Qt Creator support

qt5_wrap_ui(UI_GENERATED_HEADERS)
add_definitions(-DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE)

# Increase type limit for boost::variant
add_definitions(-DBOOST_MPL_CFG_NO_PREPROCESSED_HEADERS)
add_definitions(-DBOOST_MPL_LIMIT_LIST_SIZE=30)

add_definitions(-DCSV_IO_NO_THREAD)
add_definitions(-D_USE_MATH_DEFINES)
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Enable OpenMP
find_package(OpenMP)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

add_executable(${TARGET} WIN32 ${SOURCES} ${HEADERS} ${UI_GENERATED_HEADERS})

target_link_libraries(${TARGET}
    ${Boost_LOG_LIBRARY}
    ${Boost_DATE_TIME_LIBRARY}
    ${HDF5_LIBRARIES}
    ${NETCDF_LIBRARIES}
    ${QT5_LIBRARIES}
    ${QWT_LIBRARIES}
    ${QWTPOLAR_LIBRARIES}
    ${FMT_LIBRARIES})
