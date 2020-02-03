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

#include "StandardPlot.h"

#include <QApplication>
#include <QDebug>

#include <qwt_picker_machine.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_widget.h>
#include <qwt_symbol.h>

class ScaleDraw : public QwtScaleDraw
{
    virtual QwtText label(double value) const override
    {
        // Disable scientific notation.
        return QwtText(QString::number(value, 'f', 0));
    }
};


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
        setAxisScaleDraw(axis, new ScaleDraw);
        axisScaleDraw(axis)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
        QFont axisFont = QApplication::font();
        //axisFont.setPointSize(8);
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

StandardPlot::~StandardPlot()
{}

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
    //axisTitleFont.setPointSize(8);
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
    if (on && m_magnifier == nullptr) {
        m_magnifier = new QwtPlotMagnifier(this->canvas());
        m_magnifier->setEnabled(on);
    }
    if (on && m_panner == nullptr) {
        m_panner = new QwtPlotPanner(this->canvas());
        m_panner->setEnabled(on);
    }
}

void StandardPlot::setCurveTracker(bool on)
{
    // initialize on first use
    if (on && m_curveTracker == nullptr) {
        m_curveTracker = new PlotCurveTracker(this->canvas());
        m_curveTracker->setRubberBandPen(QPen(QColor(Qt::black)));
    }

    // Use appropriate cursor type
    if (on)
        m_canvas->setCursor(Qt::CrossCursor);
    else
        m_canvas->setCursor(Qt::ArrowCursor);

    m_curveTracker->setEnabled(on);
}

void StandardPlot::setItemTitleTracker(bool on)
{
    // initialize on first use
    if (on && m_titleTracker == nullptr) {
        m_titleTracker = new PlotItemTitleTracker(this->canvas());
    }

    m_titleTracker->setEnabled(on);
}


void StandardPlot::setRescaler(bool on)
{
    // initialize on first use
    if (on && m_rescaler == nullptr) {
        m_rescaler = new QwtPlotRescaler(this->canvas());
        m_rescaler->setRescalePolicy(QwtPlotRescaler::Expanding);
        m_rescaler->setExpandingDirection(QwtPlotRescaler::ExpandBoth);
        m_rescaler->setReferenceAxis(QwtPlot::xBottom);
        m_rescaler->setAspectRatio(QwtPlot::xBottom, 1.0);
        m_rescaler->setAspectRatio(QwtPlot::yLeft, 1.0);
        m_rescaler->setAspectRatio(QwtPlot::yRight, 0.0);
        m_rescaler->setAspectRatio(QwtPlot::xTop, 0.0);
    }

    m_rescaler->setEnabled(on);
    updateAxes();
}

void StandardPlot::addPoint(const QPointF& point, const int size, const QBrush& brush)
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

void StandardPlot::addPoints(const QPolygonF& ring, const int size, const QBrush& brush)
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

void StandardPlot::addRing(const QPolygonF& ring, const QPen& pen, const QBrush& brush, const QString& title)
{
    QwtPlotShapeItem *item = new QwtPlotShapeItem;
    item->setPolygon(ring);
    item->setPen(pen);
    item->setBrush(brush);
    item->setTitle(title);
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
    QFont font = QApplication::font();
    reflabel.setFont(font);
    refline->setLabel(reflabel);
    refline->setLabelAlignment(Qt::AlignRight);

    refline->attach(this);
}

void StandardPlot::autoScale()
{
    QwtInterval intv[axisCnt];
    const QwtPlotItemList &items = itemList();
    if (items.empty())
        return;

    // Calculate union of item bounding rects.
    QwtPlotItemIterator it;
    for (it = items.begin(); it != items.end(); ++it) {
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
    if (m_rescaler != nullptr && m_rescaler->isEnabled())
    {
        if (intv[QwtPlot::xBottom].width() > intv[QwtPlot::yLeft].width())
            m_rescaler->setReferenceAxis(QwtPlot::xBottom);
        else
            m_rescaler->setReferenceAxis(QwtPlot::yLeft);

        for (int axisId = 0; axisId < axisCnt; axisId++) {
            if (intv[axisId].isValid()) {
                m_rescaler->setIntervalHint(axisId, intv[axisId]);
            }
        }

        m_rescaler->rescale();
    }
}
