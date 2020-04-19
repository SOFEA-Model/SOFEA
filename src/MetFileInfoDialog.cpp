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

#include <algorithm>
#include <array>

#include <boost/histogram.hpp>

#include <QBoxLayout>
#include <QButtonGroup>
#include <QDateTimeEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QListView>
#include <QPainter>
#include <QPointF>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStandardItemModel>

#include <QDebug>

#include <qwt_series_data.h>
#include <qwt_polar_grid.h>
#include <qwt_scale_engine.h>
#include <qwt_painter.h>

#include "MetFileInfoDialog.h"
#include "PixmapUtilities.h"
#include "ctk/ctkRangeSlider.h"
//#include "delegates/GradientItemDelegate.h"
#include "widgets/ReadOnlyLineEdit.h"
#include "widgets/StandardTableView.h"

//-----------------------------------------------------------------------------
// WindRoseSector
//-----------------------------------------------------------------------------

void WindRoseSector::setBrush(const QBrush &brush)
{
    if (brush != d_brush) {
        d_brush = brush;
        emit itemChanged();
    }
}

const QBrush& WindRoseSector::brush() const
{
    return d_brush;
}

void WindRoseSector::drawCurve(QPainter *painter, int style,
    const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
    const QPointF &pole, int from, int to) const
{
    switch (style)
    {
    case Lines:
        drawLines(painter, azimuthMap, radialMap, pole, from, to);
        break;
    case UserCurve:
        drawSector(painter, azimuthMap, radialMap, pole, from, to);
        break;
    case NoCurve:
    default:
        break;
    }
}

void WindRoseSector::drawSector(QPainter *painter,
    const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
    const QPointF &pole, int from, int to) const
{
    // Draw a custom curve style consisting of a filled annular sector
    // bounded by 2 concentric circles and 2 lines. Assumes that the
    // vertex order is counter-clockwise starting from the minimum radius
    // and azimuth.

    const int n = to - from + 1;
    if (n != 2)
        return;

    const QwtPointPolar a0r0 = sample(0);
    const QwtPointPolar a1r1 = sample(1);

    const double r0 = radialMap.transform(a0r0.radius());
    const double a0 = azimuthMap.transform(a0r0.azimuth());
    const double r1 = radialMap.transform(a1r1.radius());
    const double a1 = azimuthMap.transform(a1r1.azimuth());

    QRectF rect0, rect1;
    rect0.setSize(QSizeF(r0 * 2, r0 * 2));
    rect1.setSize(QSizeF(r1 * 2, r1 * 2));
    rect0.moveCenter(pole);
    rect1.moveCenter(pole);

    const double theta = qRadiansToDegrees(a1 - a0);

    QPainterPath path;
    path.arcMoveTo(rect0, qRadiansToDegrees(a0));
    path.arcTo(rect0, qRadiansToDegrees(a0), theta);
    path.lineTo(qwtPolar2Pos(pole, r1, a1));
    path.arcTo(rect1, qRadiansToDegrees(a1), -theta);
    path.closeSubpath();

    painter->setBrush(brush());
    painter->setPen(pen());
    QwtPainter::drawPath(painter, path);
}

//-----------------------------------------------------------------------------
// WindRosePlot
//-----------------------------------------------------------------------------

WindRosePlot::WindRosePlot(QWidget *parent) : QwtPolarPlot(parent)
{
    setAutoReplot(false);
    setMinimumSize(600, 600);
    setPlotBackground(Qt::white);

    const QwtInterval radialInterval(-0.01, 0.2);
    const QwtInterval azimuthInterval(0.0, 360.0);

    setScale(QwtPolar::Azimuth,
        azimuthInterval.minValue(), azimuthInterval.maxValue(),
        azimuthInterval.width() / 12);

    setScaleMaxMinor(QwtPolar::Azimuth, 2);
    setScale(QwtPolar::Radius,
        radialInterval.minValue(), radialInterval.maxValue());

    // Grid and Axes
    d_grid = new QwtPolarGrid();
    d_grid->setPen(QPen(Qt::black));

    for (int scaleId=0; scaleId < QwtPolar::ScaleCount; ++scaleId) {
        d_grid->showGrid(scaleId);
        d_grid->showMinorGrid(scaleId);
        QPen minorPen(Qt::gray);
        minorPen.setStyle(Qt::DotLine);
        d_grid->setMinorGridPen(scaleId, minorPen);
    }

    d_grid->setAxisPen(QwtPolar::AxisAzimuth, QPen(Qt::black));
    d_grid->showAxis(QwtPolar::AxisAzimuth, true);
    d_grid->showAxis(QwtPolar::AxisLeft, false);
    d_grid->showAxis(QwtPolar::AxisRight, true);
    d_grid->showAxis(QwtPolar::AxisTop, true);
    d_grid->showAxis(QwtPolar::AxisBottom, false);
    d_grid->showGrid(QwtPolar::Azimuth, true);
    d_grid->showGrid(QwtPolar::Radius, true);
    d_grid->setZ(1000);

    d_grid->attach(this);
}

//-----------------------------------------------------------------------------
// PolarPointSeriesData
//-----------------------------------------------------------------------------

class PolarPointSeriesData : public QwtArraySeriesData<QwtPointPolar>
{
public:
    PolarPointSeriesData(const QVector<QwtPointPolar>& samples)
    {
        setSamples(samples);
    }

    void setSamples(const QVector<QwtPointPolar>& samples)
    {
        d_samples.clear();
        d_boundingRect = QRectF();
        if (samples.count()) {
            QwtInterval xInterval = QwtInterval(samples.at(0).azimuth(), samples.at(0).azimuth());
            QwtInterval yInterval = QwtInterval(samples.at(0).radius(), samples.at(0).radius());
            for (QwtPointPolar sample : samples) {
                d_samples << sample;
                xInterval |= sample.azimuth();
                yInterval |= sample.radius();
            }
            d_boundingRect = QRectF(xInterval.minValue(), yInterval.minValue(), xInterval.width(), yInterval.width());
        }
    }

    QRectF boundingRect() const
    {
        return d_boundingRect;
    }
};

//-----------------------------------------------------------------------------
// MetFileInfoDialog
//-----------------------------------------------------------------------------

MetFileInfoDialog::MetFileInfoDialog(std::shared_ptr<SurfaceData> sd, QWidget *parent)
    : QDialog(parent), sd(sd)
{
    setWindowTitle("Diagnostics");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Data Controls
    timeRangeSlider = new ctkRangeSlider(Qt::Horizontal);
    timeRangeSlider->setEnabled(false);

    dteMinTime = new QDateTimeEdit;
    dteMinTime->setTimeSpec(Qt::UTC);
    dteMinTime->setDisplayFormat("yyyy-MM-dd HH:mm");
    dteMinTime->setEnabled(false);

    dteMaxTime = new QDateTimeEdit;
    dteMaxTime->setTimeSpec(Qt::UTC);
    dteMaxTime->setDisplayFormat("yyyy-MM-dd HH:mm");
    dteMaxTime->setEnabled(false);

    leTotalHours = new ReadOnlyLineEdit;
    leCalmHours = new ReadOnlyLineEdit;
    leMissingHours = new ReadOnlyLineEdit;

    // Plot Widget
    wrPlot = new WindRosePlot;

    // Plot Controls
    rbSectorSize10 = new QRadioButton(QLatin1String("10\x00b0"));
    rbSectorSize15 = new QRadioButton(QLatin1String("15\x00b0"));
    rbSectorSize30 = new QRadioButton(QLatin1String("30\x00b0"));
    bgSectorSize = new QButtonGroup;
    bgSectorSize->addButton(rbSectorSize10);
    bgSectorSize->addButton(rbSectorSize15);
    bgSectorSize->addButton(rbSectorSize30);
    sbBinCount = new QSpinBox;
    sbBinCount->setRange(1, 6);
    sbBinCount->setValue(6);

    // TEST: Gradient Selector
    /*
    QComboBox *cboGradient = new QComboBox;
    cboGradient->setIconSize(QSize(48, 16));
    QStandardItemModel *gradientModel = new QStandardItemModel;
    cboGradient->setModel(gradientModel);
    cboGradient->setItemDelegate(new GradientItemDelegate);

    QStandardItem *item1 = new QStandardItem;
    QLinearGradient fade1(0, 0, 48, 16);
    fade1.setColorAt(0, QColor(0, 0, 0, 255));
    fade1.setColorAt(1, QColor(0, 0, 0, 0));
    item1->setData("TEST", Qt::DisplayRole);
    item1->setData(QVariant::fromValue(fade1), Qt::DecorationRole);
    gradientModel->setItem(0, 0, item1);

    QStandardItem *item2 = new QStandardItem;
    QLinearGradient fade2(0, 0, 48, 16);
    fade2.setColorAt(0, QColor(255, 0, 0, 255));
    fade2.setColorAt(1, QColor(0, 0, 255, 255));
    item2->setData("TEST", Qt::DisplayRole);
    item2->setData(QVariant::fromValue(fade2), Qt::DecorationRole);
    gradientModel->setItem(1, 0, item2);
    */

    // Bin Editor Model
    binModel = new QStandardItemModel;

    // Bin Editor
    binView = new QListView;
    binView->setModel(binModel);
    binView->setFrameShape(QFrame::NoFrame);
    binView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    binView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    binView->setMovement(QListView::Static);
    binView->setResizeMode(QListView::Adjust);
    binView->setFlow(QListView::LeftToRight);
    binView->setWrapping(true);
    binView->setIconSize(QSize(16,16));
    binView->setFixedHeight(16+5+5); // FIXME, subclass and set sizeHint
    // https://stackoverflow.com/questions/25613456/adjust-the-height-of-qlistview-to-fit-the-content

    // Radio Button Layout
    QHBoxLayout *sectorSizeLayout = new QHBoxLayout;
    sectorSizeLayout->addWidget(rbSectorSize10);
    sectorSizeLayout->addWidget(rbSectorSize15);
    sectorSizeLayout->addWidget(rbSectorSize30);
    sectorSizeLayout->addStretch(1);

    // Controls Layout
    QGridLayout *controlsLayout = new QGridLayout;
    controlsLayout->setColumnMinimumWidth(2, 10);
    controlsLayout->setColumnMinimumWidth(5, 10);
    controlsLayout->setColumnStretch(1, 2);
    controlsLayout->setColumnStretch(4, 0);
    // Time Controls
    controlsLayout->addWidget(new QLabel(tr("Interval: ")),           0, 0);
    controlsLayout->addWidget(timeRangeSlider,                        0, 1);
    controlsLayout->addWidget(new QLabel(tr("Start time:")),          1, 0);
    controlsLayout->addWidget(dteMinTime,                             1, 1);
    controlsLayout->addWidget(new QLabel(tr("End time:")),            2, 0);
    controlsLayout->addWidget(dteMaxTime,                             2, 1);
    // Time Metrics
    controlsLayout->addWidget(new QLabel(tr("Total hours: ")),        0, 3);
    controlsLayout->addWidget(leTotalHours,                           0, 4);
    controlsLayout->addWidget(new QLabel(tr("Calm hours: ")),         1, 3);
    controlsLayout->addWidget(leCalmHours,                            1, 4);
    controlsLayout->addWidget(new QLabel(tr("Missing hours: ")),      2, 3);
    controlsLayout->addWidget(leMissingHours,                         2, 4);
    // Plot Controls
    controlsLayout->addWidget(new QLabel(tr("Sector size: ")),        0, 6);
    controlsLayout->addLayout(sectorSizeLayout,                       0, 7);
    controlsLayout->addWidget(new QLabel(tr("Bin count: ")),          1, 6);
    controlsLayout->addWidget(sbBinCount,                             1, 7);

    // TODO:
    // Azimuth Spacing (degrees)
    // Automatic Radius, or Min/Max
    // Min/Max Wind Speed

    // Controls Frame Layout
    QVBoxLayout *frameLayout = new QVBoxLayout;
    frameLayout->addLayout(controlsLayout);
    frameLayout->addSpacing(10);
    frameLayout->addWidget(binView);

    // Controls Frame
    QFrame *controlsFrame = new QFrame;
    controlsFrame->setBackgroundRole(QPalette::Base);
    controlsFrame->setAutoFillBackground(true);
    controlsFrame->setFrameShape(QFrame::NoFrame);
    controlsFrame->setLayout(frameLayout);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(controlsFrame, 0);
    mainLayout->addWidget(wrPlot, 1);
    setLayout(mainLayout);

    // Default Color Map
    wrColors.push_back(QColor(0,   41,  255, 192));
    wrColors.push_back(QColor(0,   213, 255, 192));
    wrColors.push_back(QColor(125, 255, 122, 192));
    wrColors.push_back(QColor(255, 230, 0,   192));
    wrColors.push_back(QColor(255, 71,  0,   192));
    wrColors.push_back(QColor(128, 0,   0,   192));

    // Defaults
    rbSectorSize15->setChecked(true);
    sbBinCount->setValue(6);

    init();
}

void MetFileInfoDialog::init()
{
    if (sd == nullptr)
        return;

    if (sd->records.size() == 0)
        return;

    connect(timeRangeSlider, &ctkRangeSlider::valuesChanged,
            this, &MetFileInfoDialog::onSliderChanged);

    connect(bgSectorSize, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &MetFileInfoDialog::onSectorSizeChanged);

    connect(sbBinCount, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MetFileInfoDialog::onBinCountChanged);

    // Slider Configuration
    timeRangeSlider->setRange(1, sd->nrec);
    timeRangeSlider->setPositions(1, sd->nrec);
    timeRangeSlider->setEnabled(true);

    auto sr0 = sd->records.front();
    auto sr1 = sd->records.back();

    QDateTime minTime = QDateTime(QDate(sr0.mpyr, sr0.mpcmo, sr0.mpcdy),
                                  QTime(sr0.j - 1, 0, 0), Qt::UTC);
    QDateTime maxTime = QDateTime(QDate(sr1.mpyr, sr1.mpcmo, sr1.mpcdy),
                                  QTime(sr1.j - 1, 0, 0), Qt::UTC);

    dteMinTime->setDateTime(minTime);
    dteMinTime->setDateTimeRange(minTime, maxTime);
    dteMinTime->setEnabled(true);

    dteMaxTime->setDateTime(maxTime);
    dteMaxTime->setDateTimeRange(minTime, maxTime);
    dteMaxTime->setEnabled(true);

    connect(dteMinTime, &QDateTimeEdit::dateTimeChanged,
            this, &MetFileInfoDialog::onDateTimeChanged);

    connect(dteMaxTime, &QDateTimeEdit::dateTimeChanged,
            this, &MetFileInfoDialog::onDateTimeChanged);
}

void MetFileInfoDialog::onSliderChanged(const int min, const int max)
{
    const QSignalBlocker blocker1(dteMinTime);
    const QSignalBlocker blocker2(dteMaxTime);

    if (min < 1 || max < min || max > sd->records.size())
        return;

    // Update member variables.
    idxMin = min;
    idxMax = max;

    // Recalculate metrics on surface data subset.
    int nrec = idxMax - idxMin + 1;
    int ncalm = 0;
    int nmiss = 0;
    for (int i=idxMin-1; i < idxMax; ++i) {
        if (sd->records[i].calm)
            ncalm++;
        else if (sd->records[i].miss)
            nmiss++;
    }

    leTotalHours->setText(QString::number(nrec));
    leCalmHours->setText(QString::number(ncalm));
    leMissingHours->setText(QString::number(nmiss));

    auto sr0 = sd->records[idxMin - 1];
    auto sr1 = sd->records[idxMax - 1];
    QDateTime minTime = QDateTime(QDate(sr0.mpyr, sr0.mpcmo, sr0.mpcdy),
                                  QTime(sr0.j - 1, 0, 0), Qt::UTC);
    QDateTime maxTime = QDateTime(QDate(sr1.mpyr, sr1.mpcmo, sr1.mpcdy),
                                  QTime(sr1.j - 1, 0, 0), Qt::UTC);

    dteMinTime->setDateTime(minTime);
    dteMaxTime->setDateTime(maxTime);

    drawSectors();
}

void MetFileInfoDialog::onDateTimeChanged(const QDateTime& datetime)
{
    const QSignalBlocker blocker1(dteMinTime);
    const QSignalBlocker blocker2(dteMaxTime);

    QDateTime minDT, maxDT;

    QObject* obj = sender();

    // Synchronize min and max time controls.
    if (obj == dteMinTime) {
        minDT = datetime;
        maxDT = dteMaxTime->dateTime();
        if (maxDT < minDT) {
            //dteMaxTime->setDateTime(minDT);
            maxDT = minDT;
        }
    }
    else if (obj == dteMaxTime) {
        maxDT = datetime;
        minDT = dteMinTime->dateTime();
        if (minDT > maxDT) {
            //dteMinTime->setDateTime(maxDT);
            minDT = maxDT;
        }
    }
    else {
        return;
    }

    SurfaceData::SurfaceRecord minCompare;
    const QDate minDate = minDT.date();
    minCompare.mpyr = minDate.year();
    minCompare.mpcmo = minDate.month();
    minCompare.mpcdy = minDate.day();
    minCompare.j = minDT.time().hour() + 1;

    SurfaceData::SurfaceRecord maxCompare;
    const QDate maxDate = maxDT.date();
    maxCompare.mpyr = maxDate.year();
    maxCompare.mpcmo = maxDate.month();
    maxCompare.mpcdy = maxDate.day();
    maxCompare.j = maxDT.time().hour() + 1;

    // Get iterators to the range inside the requested bounds.
    auto lower = std::upper_bound(sd->records.begin(), sd->records.end(), minCompare);
    auto upper = std::lower_bound(sd->records.begin(), sd->records.end(), maxCompare);
    if (lower == sd->records.end() || upper == sd->records.end()) {
        return;
    }

    // Update the slider.
    int pos0 = std::distance(sd->records.begin(), lower);
    int pos1 = std::distance(sd->records.begin(), upper) + 1;
    timeRangeSlider->setPositions(pos0, pos1);
}

void MetFileInfoDialog::onSectorSizeChanged(int id)
{
    Q_UNUSED(id);

    if (rbSectorSize10->isChecked())
        wdBinCount = 36;
    else if (rbSectorSize15->isChecked())
        wdBinCount = 24;
    else if (rbSectorSize30->isChecked())
        wdBinCount = 12;

    drawSectors();
}

void MetFileInfoDialog::onBinCountChanged(int value)
{
    // TODO
}

void MetFileInfoDialog::drawSectors()
{
    namespace bh = boost::histogram;
    using namespace bh::literals; // enables _c suffix

    // Remove and detach existing curves.
    wrSectors.clear();
    wrPlot->detachItems(QwtPolarItem::Rtti_PolarCurve, true);

    if (idxMin < 1 || idxMax > sd->records.size())
        return;

    // Number of valid observations (denominator).
    int nvalid = 0;

    // Generate the histogram.
    auto hist = bh::make_histogram(
        bh::axis::circular<>(wdBinCount, 0.0, 360.0), // wdir (azimuth)
        bh::axis::regular<>(wsBinCount, wsMin, wsMax) // wspd (color)
    );

    for (int i = idxMin-1; i < idxMax; ++i) {
        auto sr = sd->records[i];
        if (!sr.calm && !sr.miss) {
            nvalid++;
            hist(sr.wdir, sr.wspd);
        }
    }

    if (nvalid <= 0)
        return;

    // Generate the sector curves.
    // Radius represents proportion of valid observations.
    double max_radius = 0;
    for (int i = 0; i <= hist.axis(0_c).size(); ++i)
    {
        double prev_radius = 0;
        double azimuth0 = hist.axis(0_c).bin(i).lower() + azimuthSpacing;
        double azimuth1 = hist.axis(0_c).bin(i).upper() - azimuthSpacing;

        for (int j = 0; j <= hist.axis(1_c).size(); ++j)
        {
            double radius0 = prev_radius;
            double radius1 = prev_radius + (hist.at(i, j) / static_cast<double>(nvalid));

            if (radius1 > max_radius)
                max_radius = radius1;

            // Create the series data for the sector.
            PolarPointSeriesData *series = new PolarPointSeriesData(
                { QwtPointPolar(azimuth0, radius0), QwtPointPolar(azimuth1, radius1) });

            // Get the brush associated with the wind speed bin.
            QBrush brush = QBrush(QColor(128, 128, 128, 200));
            if (j < wrColors.size())
                brush = QBrush(wrColors[j]);

            WindRoseSector *sector = new WindRoseSector;
            sector->setStyle(QwtPolarCurve::UserCurve);
            sector->setPen(QPen(Qt::black, 1));
            sector->setBrush(brush);
            sector->setData(series);
            sector->setRenderHint(WindRoseSector::RenderAntialiased);
            sector->setZ(100);
            wrSectors.push_back(sector);

            prev_radius = radius1;
        }
    }

    // Reset the radial scale, rounding up to nearest 0.01.
    double max_scale = std::ceil(max_radius * 100.0) / 100;
    wrPlot->setScale(QwtPolar::Radius, -0.01, max_scale);

    // Attach the curves.
    for (const auto& sector : wrSectors)
        sector->attach(wrPlot);

    wrPlot->replot();

    // Set legend labels for wind speed.
    for (int i = 0; i < hist.axis(1_c).size(); ++i)
    {
        double radial0 = hist.axis(1_c).bin(i).lower();
        double radial1 = hist.axis(1_c).bin(i).upper();
        QString label = tr("[")  + QString::number(radial0, 'g', 5) +
                        tr(", ") + QString::number(radial1, 'g', 5) + tr(")");

        QBrush brush = QBrush(wrColors[i]);
        QPixmap pm = PixmapUtilities::brushValuePixmap(brush);
        QStandardItem *item = new QStandardItem;
        item->setEditable(false);
        item->setData(label, Qt::DisplayRole);
        item->setData(pm, Qt::DecorationRole);
        binModel->setItem(i, 0, item);
    }
}

