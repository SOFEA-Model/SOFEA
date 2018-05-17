#include "StandardPlot.h"

#include <QApplication>

#include <qwt_picker_machine.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_shapeitem.h>
#include <qwt_scale_widget.h>
#include <qwt_symbol.h>

StandardPlot::StandardPlot(QWidget *parent) : QwtPlot(parent)
{
    // Canvas
    m_canvas = new QwtPlotCanvas;
    m_canvas->setFocusIndicator(QwtPlotCanvas::NoFocusIndicator);
    m_canvas->setCursor(Qt::ArrowCursor);
    m_canvas->setPaintAttribute(QwtPlotCanvas::BackingStore, true);
    m_canvas->setPaintAttribute(QwtPlotCanvas::Opaque, true);
    m_canvas->setFrameStyle(QFrame::Box | QFrame::Plain);
    m_canvas->setLineWidth(1);
    setCanvas(m_canvas);

    // Axis
    for(int axis=0; axis < QwtPlot::axisCnt; axis++) {
        axisWidget(axis)->setMargin(0);
        axisScaleDraw(axis)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
        QFont axisFont = QApplication::font();
        axisFont.setPointSize(9);
        setAxisFont(axis, axisFont);
    }

    // Grid
    m_grid = new QwtPlotGrid;
    m_grid->setMajorPen(Qt::gray, 0, Qt::DotLine);
    m_grid->setMinorPen(Qt::darkGray, 0, Qt::DotLine);
    m_grid->attach(this);

    // Layout
    plotLayout()->setCanvasMargin(10);
}

void StandardPlot::setXAxisTitle(const QString& text)
{
    setAxisTitleInternal(QwtPlot::xBottom, text);
}

void StandardPlot::setYAxisTitle(const QString& text)
{
    setAxisTitleInternal(QwtPlot::yLeft, text);
}

void StandardPlot::setXAxisScale(double min, double max, double step)
{
    setAxisScale(QwtPlot::xBottom, min, max, step);
    setAxisScale(QwtPlot::xTop, min, max, step);
}

void StandardPlot::setYAxisScale(double min, double max, double step)
{
    setAxisScale(QwtPlot::yLeft, min, max, step);
    setAxisScale(QwtPlot::yRight, min, max, step);
}

void StandardPlot::setAxisTitleInternal(QwtPlot::Axis axis, const QString& text)
{
    QwtText title(text);
    QFont axisTitleFont = QApplication::font();
    axisTitleFont.setPointSize(10);
    title.setFont(axisTitleFont);
    setAxisTitle(axis, title);
}

void StandardPlot::clear()
{
    detachItems(QwtPlotItem::Rtti_PlotCurve, true);
    detachItems(QwtPlotItem::Rtti_PlotShape, true);
    detachItems(QwtPlotItem::Rtti_PlotMarker, true);
}

void StandardPlot::setPanZoomMode(bool on)
{
    // initialized on first use
    if (on && (m_magnifier == nullptr || m_panner == nullptr)) {
        m_magnifier = new QwtPlotMagnifier(this->canvas());
        m_panner = new QwtPlotPanner(this->canvas());
    }

    m_magnifier->setEnabled(on);
    m_panner->setEnabled(on);
}

void StandardPlot::setCurveTracker(bool on)
{
    // initialize on first use
    if (on && m_tracker == nullptr) {
        m_tracker = new CurveTracker(this->canvas());
        m_tracker->setStateMachine(new QwtPickerTrackerMachine());
        m_tracker->setRubberBandPen(QPen(QColor(Qt::black)));
    }

    // Use appropriate cursor type
    if (on)
        m_canvas->setCursor(Qt::CrossCursor);
    else
        m_canvas->setCursor(Qt::ArrowCursor);

    m_tracker->setEnabled(on);
}

void StandardPlot::setRescaler(bool on)
{
    // initialize on first use
    if (on && m_rescaler == nullptr) {
        m_rescaler = new QwtPlotRescaler(this->canvas());
        m_rescaler->setRescalePolicy(QwtPlotRescaler::Expanding);
        m_rescaler->setExpandingDirection(QwtPlotRescaler::ExpandBoth);
        m_rescaler->setReferenceAxis(QwtPlot::xBottom);
        m_rescaler->setAspectRatio(QwtPlot::yLeft, 1.0);
        m_rescaler->setAspectRatio(QwtPlot::yRight, 0.0);
        m_rescaler->setAspectRatio(QwtPlot::xTop, 0.0);
    }

    m_rescaler->setEnabled(on);
    updateAxes();
}

void StandardPlot::addPoint(QPointF const& point, const int size, QBrush const& brush)
{
    QwtPlotMarker *marker = new QwtPlotMarker;
    marker->setValue(point);
    QwtSymbol *sym = new QwtSymbol;
    sym->setStyle(QwtSymbol::Ellipse);
    sym->setSize(size, size);
    sym->setBrush(brush);
    marker->setSymbol(sym);
    marker->attach(this);
}

void StandardPlot::addPoints(QPolygonF const& ring, const int size, QBrush const& brush)
{
    // QPolygonF is QVector<QPointF>
    for (QPointF p : ring) {
        QwtPlotMarker *marker = new QwtPlotMarker;
        marker->setValue(p);
        QwtSymbol *sym = new QwtSymbol;
        sym->setStyle(QwtSymbol::Ellipse);
        sym->setSize(size, size);
        sym->setBrush(brush);
        marker->setSymbol(sym);
        marker->attach(this);
    }
}

void StandardPlot::addRing(QPolygonF const& ring, QPen const& pen, QBrush const& brush)
{
    QwtPlotShapeItem *item = new QwtPlotShapeItem;
    item->setPolygon(ring);
    item->setPen(pen);
    item->setBrush(brush);
    item->attach(this);
}

void StandardPlot::addRefVLine(double x, double y, const QString& label)
{
    QwtPlotMarker *refline = new QwtPlotMarker;
    refline->setLineStyle(QwtPlotMarker::VLine);
    refline->setValue(x, y);

    // Label
    // TODO: make sure label is inside plotLayout()->canvasRect()
    QwtText reflabel(label);
    QFont font = reflabel.font();
    font.setPointSize(9);
    reflabel.setFont(font);
    refline->setLabel(reflabel);
    refline->setLabelAlignment(Qt::AlignRight);

    refline->attach(this);
}

void StandardPlot::autoScale()
{
    QwtInterval intv[axisCnt];

    const QwtPlotItemList &itmList = itemList();

    QwtPlotItemIterator it;
    for (it = itmList.begin(); it != itmList.end(); ++it) {
        const QwtPlotItem *item = *it;

        if (!item->isVisible())
            continue;

        const QRectF rect = item->boundingRect();
        intv[item->xAxis()] |= QwtInterval(rect.left(), rect.right());
        intv[item->yAxis()] |= QwtInterval(rect.top(), rect.bottom());
    }

    // Adjust scale for each axis.
    for (int axisId = 0; axisId < axisCnt; axisId++) {
        if (intv[axisId].isValid()) {
            double minValue = intv[axisId].minValue();
            double maxValue = intv[axisId].maxValue();
            setAxisScale(axisId, minValue, maxValue);
        }
    }

    // Recalculate scale divisions.
    updateAxes();

    // Rescale axes according to RescalePolicy, if enabled.
    if (m_rescaler != nullptr && m_rescaler->isEnabled()) {
        m_rescaler->rescale();
    }
}
