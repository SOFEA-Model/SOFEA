// Copyright 2020 Dow, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <chrono>

#include <QApplication>
#include <QBoxLayout>
#include <QCheckBox>
#include <QDesktopServices>
#include <QDir>
#include <QFormLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMetaObject>
#include <QPushButton>
#include <QProgressBar>
#include <QRectF>
#include <QStandardPaths>
#include <QTreeView>

#include <QDebug>

#include <boost/variant.hpp>
#include <boost/log/trivial.hpp>

#include "ReceptorElevationEditor.h"
#include "ReceptorVisitor.h"
#include "core/Common.h"
#include "core/TerrainProcessor.h"
#include "models/OpenCLDeviceInfoModel.h"
#include "models/WKTModel.h"
#include "widgets/BoundingBoxEditor.h"
#include "widgets/GroupBoxFrame.h"
#include "widgets/PathEdit.h"
#include "widgets/ProgressBar.h"
#include "widgets/ReadOnlyLineEdit.h"
#include "widgets/StatusLabel.h"

ReceptorElevationEditor::ReceptorElevationEditor(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(tr("Terrain Processor"));

    using namespace sofea::constants;

    QString infoURL = "<a href=\"https://www.usgs.gov/core-science-systems/ngp/3dep\">USGS 3D Elevation Program</a>";
    QString infoText = tr(
        "This utility automatically sets receptor elevations and hill-height "
        "scaling factors using elevation data from the %1 and AERMAP algorithms. "
        "Ensure the coordinate system is set correctly before using.").arg(infoURL);

    infoLabel = new StatusLabel;
    infoLabel->setStatusType(StatusLabel::InfoTip);
    infoLabel->setText(infoText);
    infoLabel->label()->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard | Qt::LinksAccessibleByMouse);

    bboxEdit = new BoundingBoxEditor;

    deviceModel = new OpenCLDeviceInfoModel(this);
    deviceView = new QTreeView;
    deviceView->setRootIsDecorated(false);
    deviceView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    deviceView->setModel(deviceModel);

    outputCheckBox = new QCheckBox(tr("Export GeoTIFF"));
    outputCheckBox->setChecked(false);

    outputPathEdit = new PathEdit;
    outputPathEdit->setDialogCaption(tr("Save File As"));
    outputPathEdit->setDialogFilter("GeoTIFF (*.tif)");
    outputPathEdit->setDialogFileMode(QFileDialog::AnyFile);
    outputPathEdit->setDialogAcceptMode(QFileDialog::AcceptSave);
    outputPathEdit->setDialogDefaultSuffix("tif");
    outputPathEdit->setEnabled(false);

    areaWarningLabel = new StatusLabel;
    areaWarningLabel->setStatusType(StatusLabel::Warning);
    areaWarningLabel->setText(tr("Bounding box is outside valid area for projection."));
    areaWarningLabel->setHidden(true);

    progressLabel = new QLabel;

    progressBar = new ProgressBar;
    progressBar->setRange(0, 100);

    btnStartStop = new QPushButton(tr("Start"));

    // Connections
    connect(infoLabel->label(), &QLabel::linkActivated, [](const QString& url) {
        QDesktopServices::openUrl(QUrl(url));
    });

    connect(bboxEdit, &BoundingBoxEditor::valueChanged,
            this, &ReceptorElevationEditor::onBoundingBoxChanged);

    connect(outputCheckBox, &QCheckBox::stateChanged, [&](int state) {
        outputPathEdit->setEnabled(state == Qt::Checked);
    });

    connect(btnStartStop, &QPushButton::clicked,
            this, &ReceptorElevationEditor::onStartStopClicked);

    // Async Event Handlers
    control_.setStartedFunction([=]() {
        this->started();
    });

    control_.setProgressFunction([=](double complete) {
        this->progress(complete);
    });

    control_.setMessageFunction([=](const std::string text) {
        this->message(QString::fromStdString(text));
    });

    control_.setFinishedFunction([=]() {
        this->finished();
    });

    connect(this, &ReceptorElevationEditor::started, this, &ReceptorElevationEditor::onStarted, Qt::QueuedConnection);
    connect(this, &ReceptorElevationEditor::progress, this, &ReceptorElevationEditor::onProgress, Qt::QueuedConnection);
    connect(this, &ReceptorElevationEditor::message, this, &ReceptorElevationEditor::onMessage, Qt::QueuedConnection);
    connect(this, &ReceptorElevationEditor::finished, this, &ReceptorElevationEditor::onFinished, Qt::QueuedConnection);

    // Layout
    QFormLayout *deviceLayout = new QFormLayout;
    deviceLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    deviceLayout->setSpacing(10);
    deviceLayout->addRow(tr("Compute Device"), deviceView);

    QFormLayout *exportLayout = new QFormLayout;
    exportLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    exportLayout->setSpacing(10);
    exportLayout->addRow(outputCheckBox, outputPathEdit);

    QHBoxLayout *updateLayout = new QHBoxLayout;
    updateLayout->addWidget(btnStartStop, 0, Qt::AlignRight);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    //mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(infoLabel);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(bboxEdit);
    mainLayout->addWidget(areaWarningLabel);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(deviceLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(exportLayout);
    mainLayout->addSpacing(15);
    mainLayout->addStretch(1);
    mainLayout->addWidget(progressLabel);
    mainLayout->addWidget(progressBar);
    mainLayout->addLayout(updateLayout);

    setLayout(mainLayout);
}

void ReceptorElevationEditor::setCompoundCRS(std::shared_ptr<PJ> ccrs)
{
    ccrs_ = ccrs;
    bboxEdit->setCompoundCRS(ccrs);
}

void ReceptorElevationEditor::setAreaOfUse(const Projection::GeographicExtent& area)
{
    validArea_ = area;
}

void ReceptorElevationEditor::setReceptors(const std::vector<ReceptorGroup>& receptors)
{
    QRectF bbox;
    for (const auto& group : receptors)
        bbox |= boost::apply_visitor(ReceptorGroupRectVisitor(), group);

    bboxEdit->setValue(bbox);
}

void ReceptorElevationEditor::setBoundingBox(const QRectF& bbox)
{
    bboxEdit->setValue(bbox);
}

void ReceptorElevationEditor::onBoundingBoxChanged(const QRectF& bbox)
{
    using namespace Projection;

    GeographicExtent area;

    if (bboxEdit->mode() == BoundingBoxEditor::Projected) {
        // Transform to geographic coordinates.
        auto pcrs = getComponentCRS(ccrs_, 0);
        auto gcrs = getGeodeticCRS(pcrs);
        auto pipeline = Pipeline(pcrs, gcrs);

        double xmin = bbox.x();
        double ymax = bbox.y();
        double xmax = xmin + bbox.width();
        double ymin = ymax - bbox.height();

        double llx = 0, lly = 0, llz = 0;
        double lrx = 0, lry = 0, lrz = 0;
        double ulx = 0, uly = 0, ulz = 0;
        double urx = 0, ury = 0, urz = 0;
        pipeline.forward(xmin, ymin, 0, llx, lly, llz); // SW corner
        pipeline.forward(xmax, ymin, 0, lrx, lry, lrz); // SE corner
        pipeline.forward(xmin, ymax, 0, ulx, uly, ulz); // NW corner
        pipeline.forward(xmax, ymax, 0, urx, ury, urz); // NE corner

        // Get the maximum area covering the projected bounding box.
        area.westLongitude = std::min(llx, ulx); // SW, NW: West
        area.southLatitude = std::min(lly, lry); // SW, SE: South
        area.eastLongitude = std::max(lrx, urx); // SE, NE: East
        area.northLatitude = std::max(uly, ury); // NW, NE: North
    }
    else {
        area.westLongitude = bbox.x();
        area.northLatitude = bbox.y();
        area.eastLongitude = area.westLongitude + bbox.width();
        area.southLatitude = area.northLatitude - bbox.height();
    }

    qDebug() << "W" << area.westLongitude;
    qDebug() << "N" << area.northLatitude;
    qDebug() << "E" << area.eastLongitude;
    qDebug() << "S" << area.southLatitude;


    // Show warning if we are outside the area of use.
    if (area.westLongitude < validArea_.westLongitude ||
        area.southLatitude < validArea_.southLatitude ||
        area.eastLongitude > validArea_.eastLongitude ||
        area.northLatitude > validArea_.northLatitude)
        areaWarningLabel->setHidden(false);
    else
        areaWarningLabel->setHidden(true);
}

void ReceptorElevationEditor::onStarted()
{
    btnStartStop->setText(tr("Abort"));
    progressBar->setState(ProgressBar::Running);
    progressBar->setValue(0);
}

void ReceptorElevationEditor::onProgress(double complete)
{
    int progress = std::clamp<int>(std::lround(complete * 100), 0, 100);
    progressBar->setValue(progress);
}

void ReceptorElevationEditor::onMessage(const QString& message)
{
    progressLabel->setText(message);
}

void ReceptorElevationEditor::onFinished()
{
    btnStartStop->setText(tr("Start"));

    try {
        std::vector<float> array = future_.get();
    } catch (const std::exception& e) {
        progressLabel->setText(e.what());
        progressBar->setState(ProgressBar::Error);
    }
}

void ReceptorElevationEditor::onStartStopClicked()
{
    using namespace sofea::constants;

    // Abort if the future is not ready.
    if (future_.valid() && future_.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready) {
        progressLabel->setText("Stopping");
        control_.requestInterrupt();
        return;
    }

    Projection::ProjectedExtent bbox;
    bbox.xmin = bboxEdit->xmin();
    bbox.ymin = bboxEdit->ymin();
    bbox.xmax = bboxEdit->xmax();
    bbox.ymax = bboxEdit->ymax();

    auto conv = Projection::createConversion("16017"); // UTM 17N
    auto gcrs = Projection::createGeodeticCRS(EPSG_HDATUM_NAD83_1986);
    auto pcrs = Projection::createProjectedCRS(gcrs, conv, EPSG_UOM_METER);
    //auto vcrs = Projection::createVerticalCRS(EPSG_VDATUM_NAVD88, EPSG_UOM_METER);
    //auto compound = Projection::createCompoundCRS(pcrs, vcrs);

    //std::string wcsFilename = "WCS:" + std::string(USGS_3DEP_WCS_ENDPOINT);
    //QString appCachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    //QString wcsCachePath = QDir::cleanPath(appCachePath + QDir::separator() + "wcs");
    //std::string wcsCacheOption = "CACHE:" + wcsCachePath.toStdString();
    //
    //try {
    //    dataset = std::make_unique<Raster::Dataset>(wcsFilename, "WCS", std::vector<std::string>{ wcsCacheOption });
    //    dataset->setBoundingBox(pcrs, bbox);
    //    future = dataset->readBlocks(control);
    //} catch (const std::exception& e) {
    //    progressLabel->setText(e.what());
    //    progressBar->setState(ProgressBar::Error);
    //    return;
    //}
}
