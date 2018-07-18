#include <QtWidgets>

#include <qwt_series_data.h>
#include <qwt_polar_grid.h>
#include <qwt_polar_curve.h>
#include <qwt_scale_engine.h>
#include <qwt_painter.h>

// Master branch as of 2018-07-17:
// https://github.com/HDembinski/histogram/tree/bff0c7bb

#include <boost/histogram.hpp>

#include "MetFileInfoDialog.h"


// Internal function from qwtpolar/src/qwt_polar_curve.cpp
static inline bool qwtInsidePole(const QwtScaleMap &map, double radius)
{
    return map.isInverting() ? (radius > map.s1()) : (radius < map.s1());
}

//-----------------------------------------------------------------------------
// WindRoseSector
//-----------------------------------------------------------------------------

void WindRoseSector::setBrush(const QBrush &brush)
{
    if (brush != d_brush) {
        d_brush = brush;
        itemChanged();
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
        drawPolygon(painter, azimuthMap, radialMap, pole, from, to);
        break;
    case NoCurve:
    default:
        break;
    }
}

void WindRoseSector::drawPolygon(QPainter *painter,
    const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
    const QPointF &pole, int from, int to) const
{
    int size = to - from + 1;
    if (size <= 0)
        return;

    QPolygonF polyline;
    polyline.resize(size);
    QPointF *polylineData = polyline.data();

    for (int i = from; i <= to; ++i)
    {
        QwtPointPolar point = sample(i);
        if (!qwtInsidePole(radialMap, point.radius())) {
            double r = radialMap.transform(point.radius());
            const double a = azimuthMap.transform(point.azimuth());
            polylineData[i - from] = qwtPolar2Pos(pole, r, a);
        }
        else {
            polylineData[i - from] = pole;
        }
    }

    painter->setBrush(brush());
    painter->setPen(pen());
    QwtPainter::drawPolygon(painter, polyline);
}

//-----------------------------------------------------------------------------
// WindRosePlot
//-----------------------------------------------------------------------------

WindRosePlot::WindRosePlot(QWidget *parent) : QwtPolarPlot(parent)
{
    setAutoReplot(false);
    setPlotBackground(Qt::white);
    setMinimumSize(600, 600);

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
    d_grid->setPen(QPen(Qt::darkGray));

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

    d_grid->attach(this);
}

//-----------------------------------------------------------------------------
// PolarPointSeriesData
//-----------------------------------------------------------------------------

class PolarPointSeriesData : public QwtArraySeriesData<QwtPointPolar>
{
public:
    PolarPointSeriesData(QVector<QwtPointPolar> samples)
    {
        setSamples(samples);
    }

    void setSamples(QVector<QwtPointPolar> samples)
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
    setWindowFlag(Qt::WindowMaximizeButtonHint);

    // Data Controls
    timeRangeSlider = new ctkRangeSlider(Qt::Horizontal);
    timeRangeSlider->setEnabled(false);
    leStartTime = new ReadOnlyLineEdit;
    leEndTime = new ReadOnlyLineEdit;
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

    // Bin Editor Model
    binModel = new QStandardItemModel;

    // Bin Editor
    binEditor = new QListView;
    binEditor->setModel(binModel);
    binEditor->setFrameShape(QFrame::NoFrame);
    binEditor->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    binEditor->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    binEditor->setMovement(QListView::Static);
    binEditor->setResizeMode(QListView::Adjust);
    binEditor->setFlow(QListView::LeftToRight);
    binEditor->setWrapping(true);
    binEditor->setIconSize(QSize(16,16));
    binEditor->setFixedHeight(16+5+5); // FIXME, subclass and set sizeHint
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
    controlsLayout->addWidget(leStartTime,                            1, 1);
    controlsLayout->addWidget(new QLabel(tr("End time:")),            2, 0);
    controlsLayout->addWidget(leEndTime,                              2, 1);
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

    // Azimuth Spacing (degrees)
    // Automatic Radius, or Min/Max
    // Min/Max Wind Speed

    // Controls Frame Layout
    QVBoxLayout *frameLayout = new QVBoxLayout;
    frameLayout->addLayout(controlsLayout);
    frameLayout->addSpacing(10);
    frameLayout->addWidget(binEditor);

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

    if (sd == nullptr)
        return;

    if (sd->records.size() == 0)
        return;

    init();
}

void MetFileInfoDialog::init()
{
    // Default Color Map
    wrColors.push_back(QColor(0,   41,  255, 200));
    wrColors.push_back(QColor(0,   213, 255, 200));
    wrColors.push_back(QColor(125, 255, 122, 200));
    wrColors.push_back(QColor(255, 230, 0,   200));
    wrColors.push_back(QColor(255, 71,  0,   200));
    wrColors.push_back(QColor(128, 0,   0,   200));

    // Defaults
    rbSectorSize15->setChecked(true);
    sbBinCount->setValue(6);

    // Connections
    connect(timeRangeSlider, &ctkRangeSlider::valuesChanged,
            this, &MetFileInfoDialog::onIntervalChanged);

    connect(bgSectorSize, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &MetFileInfoDialog::onSectorSizeChanged);

    connect(sbBinCount, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MetFileInfoDialog::onBinCountChanged);

    timeRangeSlider->setRange(1, sd->nrec);
    timeRangeSlider->setPositions(1, sd->nrec);
    timeRangeSlider->setEnabled(true);
}

void MetFileInfoDialog::onIntervalChanged(const int min, const int max)
{
    if (min < 1 || max > sd->records.size())
        return;

    // Update member variables.
    idxMin = min;
    idxMax = max;

    // Recalculate derived values on surface data subset.
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

    const QString dateFormat = "yyyy-MM-dd HH:mm";
    leStartTime->setText(minTime.toString(dateFormat));
    leEndTime->setText(maxTime.toString(dateFormat));

    drawSectors();
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

}

void MetFileInfoDialog::drawSectors()
{
    namespace bh = boost::histogram;
    using namespace bh::literals; // enables _c suffix

    if (idxMin < 1 || idxMax > sd->records.size())
        return;

    // Remove and delete existing curves.
    wrSectors.clear();
    wrPlot->detachItems(QwtPolarItem::Rtti_PolarCurve, true);

    // Number of valid observations (denominator).
    int nvalid = 0;

    // Generate the histogram.
    using wdir_axis_t = bh::axis::circular<>;
    using wspd_axis_t = bh::axis::regular<>;
    auto hist = bh::make_static_histogram(
        wdir_axis_t(wdBinCount, 0.0, 360.0, "wdir"),  // wdir (azimuth)
        wspd_axis_t(wsBinCount, wsMin, wsMax, "wspd") // wspd (color)
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

    for (auto wdir : hist.axis(0_c))
    {
        double prev_radius = 0;

        for (auto wspd : hist.axis(1_c))
        {
            double azimuth0 = wdir.lower() + azimuthSpacing;
            double azimuth1 = wdir.upper() - azimuthSpacing;
            double radius0 = prev_radius;
            double radius1 = prev_radius + (hist.at(wdir, wspd).value() /
                                            static_cast<double>(nvalid));

            if (radius1 > max_radius)
                max_radius = radius1;

            // Create the point series for Qwt.
            QVector<QwtPointPolar> vertices;
            vertices.push_back(QwtPointPolar(azimuth0, radius0));
            vertices.push_back(QwtPointPolar(azimuth0, radius1));
            vertices.push_back(QwtPointPolar(azimuth1, radius1));
            vertices.push_back(QwtPointPolar(azimuth1, radius0));
            vertices.push_back(QwtPointPolar(azimuth0, radius0));
            PolarPointSeriesData *series = new PolarPointSeriesData(vertices);

            // Get the brush associated with the bin.
            QBrush brush = QBrush(QColor(128, 128, 128, 200));
            if (wspd.idx() < wrColors.size())
                brush = QBrush(wrColors[wspd.idx()]);

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

    // Legend labels for wind speed.
    for (auto wspd : hist.axis(1_c))
    {
        double radial0 = wspd.lower();
        double radial1 = wspd.upper();
        QString label = tr("[")  + QString::number(radial0, 'g', 5) +
                        tr(", ") + QString::number(radial1, 'g', 5) + tr(")");

        QBrush brush = QBrush(wrColors[wspd.idx()]);
        QPixmap pm = ColorPickerDelegate::brushValuePixmap(brush);
        QStandardItem *item = new QStandardItem;
        item->setEditable(false);
        item->setData(label, Qt::DisplayRole);
        item->setData(pm, Qt::DecorationRole);
        binModel->setItem(wspd.idx(), 0, item);
    }
}

