#include <QtWidgets>

#include <sstream>
#include <iostream> // DEBUG

#include <qwt_series_data.h>
#include <qwt_polar_grid.h>
#include <qwt_polar_curve.h>
#include <qwt_scale_engine.h>
#include <qwt_painter.h>

//#include <boost/date_time/gregorian/gregorian.hpp>
//#include <boost/date_time/posix_time/posix_time.hpp>

// https://github.com/HDembinski/histogram/tree/v2.0
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4068) // Level 1, C4068: unknown pragma
#include <boost/histogram.hpp>
#pragma warning(pop)
#endif

#include "MetFileInfoDialog.h"

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

    // Plot Controls
    rbSectorSize10 = new QRadioButton(QLatin1String("10\x00b0"));
    rbSectorSize15 = new QRadioButton(QLatin1String("15\x00b0"));
    rbSectorSize30 = new QRadioButton(QLatin1String("30\x00b0"));
    bgSectorSize = new QButtonGroup;
    bgSectorSize->addButton(rbSectorSize10);
    bgSectorSize->addButton(rbSectorSize15);
    bgSectorSize->addButton(rbSectorSize30);
    //sbBinCount = new QSpinBox;
    //sbBinCount->setRange(1, 10);

    // Plot Widgets
    wrPlot = new WindRosePlot;

    // Color Map
    colors.push_back(QColor(0,   41,  255, 200));
    colors.push_back(QColor(0,   213, 255, 200));
    colors.push_back(QColor(125, 255, 122, 200));
    colors.push_back(QColor(255, 230, 0,   200));
    colors.push_back(QColor(255, 71,  0,   200));
    colors.push_back(QColor(128, 0,   0,   200));

    // Controls Layout
    QFormLayout *layout1 = new QFormLayout;
    layout1->addRow(new QLabel(tr("Interval: ")), timeRangeSlider);
    layout1->addRow(new QLabel(tr("Start time:")), leStartTime);
    layout1->addRow(new QLabel(tr("End time:")), leEndTime);

    QFormLayout *layout2 = new QFormLayout;
    layout2->addRow(new QLabel(tr("Total hours: ")), leTotalHours);
    layout2->addRow(new QLabel(tr("Calm hours: ")), leCalmHours);
    layout2->addRow(new QLabel(tr("Missing hours: ")), leMissingHours);

    QHBoxLayout *dataControlsLayout = new QHBoxLayout;
    dataControlsLayout->addLayout(layout1);
    dataControlsLayout->addSpacing(5);
    dataControlsLayout->addLayout(layout2);

    QHBoxLayout *plotControlsLayout = new QHBoxLayout;
    plotControlsLayout->addWidget(new QLabel(tr("Sector size: ")));
    plotControlsLayout->addWidget(rbSectorSize10);
    plotControlsLayout->addWidget(rbSectorSize15);
    plotControlsLayout->addWidget(rbSectorSize30);
    plotControlsLayout->addStretch(1);

    QVBoxLayout *controlsLayout = new QVBoxLayout;
    controlsLayout->addLayout(dataControlsLayout);
    controlsLayout->addSpacing(10);
    controlsLayout->addLayout(plotControlsLayout);

    QFrame *controlsFrame = new QFrame;
    controlsFrame->setBackgroundRole(QPalette::Base);
    controlsFrame->setAutoFillBackground(true);
    controlsFrame->setFrameShape(QFrame::NoFrame);
    controlsFrame->setLayout(controlsLayout);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(controlsFrame);
    mainLayout->addWidget(wrPlot);

    setLayout(mainLayout);
    init();
}

void MetFileInfoDialog::init()
{
    if (sd == nullptr)
        return;

    if (sd->records.size() == 0)
        return;

    // Defaults
    rbSectorSize15->setChecked(true);

    // Connections
    connect(timeRangeSlider, &ctkRangeSlider::valuesChanged,
            this, &MetFileInfoDialog::onRangeChanged);

    connect(bgSectorSize, QOverload<int>::of(&QButtonGroup::buttonClicked),
        [=](int id) {
        if (rbSectorSize10->isChecked())
            wdBinCount = 36;
        else if (rbSectorSize15->isChecked())
            wdBinCount = 24;
        else if (rbSectorSize30->isChecked())
            wdBinCount = 12;

        drawSectors();
    });

    timeRangeSlider->setRange(1, sd->nrec);
    timeRangeSlider->setPositions(1, sd->nrec);
    timeRangeSlider->setEnabled(true);
}

void MetFileInfoDialog::onRangeChanged(const int min, const int max)
{
    if (min < 1 || max > sd->records.size())
        return;

    // Update member variables.
    idxMin = min;
    idxMax = max;

    // Recalculate derived values on subset.
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

void MetFileInfoDialog::drawSectors()
{
    namespace bh = boost::histogram;
    using namespace bh::literals; // for _c
    typedef bh::histogram<bh::Dynamic, bh::builtin_axes, bh::adaptive_storage<>> histogram_t;

    if (idxMin < 1 || idxMax > sd->records.size())
        return;

    // Remove and delete existing curves.
    sectors.clear();
    wrPlot->detachItems(QwtPolarItem::Rtti_PolarCurve, true);

    // Number of valid observations (denominator).
    int nvalid = 0;

    // Generate the histogram.
    std::vector<histogram_t::axis_type> axes;
    axes.emplace_back(bh::axis::circular<>(wdBinCount, 0.0, 360.0, "wdir"));  // wdir (azimuth)
    axes.emplace_back(bh::axis::regular<>(wsBinCount, wsMin, wsMax, "wspd")); // wspd (color)
    histogram_t h = histogram_t(axes.begin(), axes.end());

    for (int i=idxMin-1; i < idxMax; ++i) {
        auto sr = sd->records[i];
        if (!sr.calm && !sr.miss) {
            nvalid++;
            h.fill(sr.wdir, sr.wspd);
        }
    }

    if (nvalid <= 0)
        return;

    // Generate the sector curves.
    // Radius represents proportion of valid observations.

    double max_radius = 0;

    for (int i=0; i < bh::bins(h.axis(0_c)); ++i)
    {
        double prev_radius = 0;

        for (int j=0; j <= bh::bins(h.axis(1_c)); ++j)
        {
            double azimuth0 = bh::left(h.axis(0_c), i) + azimuthSpacing;
            double azimuth1 = bh::right(h.axis(0_c), i) - azimuthSpacing;
            double radius0 = prev_radius;
            double radius1 = prev_radius + (h.value(i, j) / static_cast<double>(nvalid));

            if (radius1 > max_radius)
                max_radius = radius1;

            QVector<QwtPointPolar> vertices;
            vertices.push_back(QwtPointPolar(azimuth0, radius0));
            vertices.push_back(QwtPointPolar(azimuth0, radius1));
            vertices.push_back(QwtPointPolar(azimuth1, radius1));
            vertices.push_back(QwtPointPolar(azimuth1, radius0));
            vertices.push_back(QwtPointPolar(azimuth0, radius0));
            PolarPointSeriesData *series = new PolarPointSeriesData(vertices);

            // Get the brush associated with the bin.
            QBrush brush = QBrush(QColor(128, 128, 128, 200));
            if (j < colors.size())
                brush = QBrush(colors[j]);

            WindRoseSector *sector = new WindRoseSector;
            sector->setStyle(QwtPolarCurve::UserCurve);
            sector->setPen(QPen(Qt::black, 1));
            sector->setBrush(brush);
            sector->setData(series);
            sector->setRenderHint(WindRoseSector::RenderAntialiased);
            sectors.push_back(sector);

            prev_radius = radius1;
        }
    }

    // Reset the radial scale, rounding up to nearest 0.01.
    double max_scale = std::ceil(max_radius * 100.0) / 100;
    wrPlot->setScale(QwtPolar::Radius, -0.01, max_scale);

    // Attach the curves.
    for (const auto& sector : sectors)
        sector->attach(wrPlot);

    wrPlot->replot();

    // Legend labels for wind speed.
    for (int j=0; j < bh::bins(h.axis(1_c)); ++j)
    {
        double radial0 = bh::left(h.axis(1_c), j);
        double radial1 = bh::right(h.axis(1_c), j);
        //std::ostringstream oss;
        //oss << "[" << radial0 << ", " << radial1 << ")";
        //std::cout << oss.str();
    }
}

