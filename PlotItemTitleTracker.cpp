#include "PlotItemTitleTracker.h"

#include <QApplication>

#include <qwt_picker_machine.h>
#include <qwt_plot.h>
#include <qwt_plot_item.h>
#include <qwt_plot_shapeitem.h>

PlotItemTitleTracker::PlotItemTitleTracker(QWidget *canvas) : QwtPlotPicker(canvas)
{
    setTrackerMode(QwtPlotPicker::AlwaysOn);
    setRubberBand(NoRubberBand);
    setStateMachine(new QwtPickerDragPointMachine());
}

QwtText PlotItemTitleTracker::trackerText(const QPoint &pos) const
{
    const QwtPlotShapeItem *item = itemAt(pos);
    if (item == nullptr)
        return QwtText();

    QwtText trackerText = item->title();
    QFont trackerFont = QApplication::font();
    trackerFont.setPointSize(9);
    trackerText.setFont(trackerFont);
    QColor foregroundColor = QApplication::palette().color(QPalette::WindowText);
    QColor backgroundColor = QApplication::palette().color(QPalette::Base);
    trackerText.setColor(foregroundColor);
    trackerText.setBackgroundBrush(QBrush(backgroundColor));
    return trackerText;
}

QwtPlotShapeItem* PlotItemTitleTracker::itemAt(const QPoint& pos) const
{
    const QwtPlot *plot = this->plot();
    if (plot == nullptr)
        return nullptr;

    // Translate pos into plot coordinates
    double coords[QwtPlot::axisCnt];
    coords[QwtPlot::xBottom] = plot->canvasMap(QwtPlot::xBottom).invTransform(pos.x());
    coords[QwtPlot::xTop] = plot->canvasMap(QwtPlot::xTop).invTransform(pos.x());
    coords[QwtPlot::yLeft] = plot->canvasMap(QwtPlot::yLeft).invTransform(pos.y());
    coords[QwtPlot::yRight] = plot->canvasMap( QwtPlot::yRight).invTransform(pos.y());

    // Items are in increasing Z-order. Return the top item.
    QwtPlotItemList items = plot->itemList(QwtPlotItem::Rtti_PlotShape);
    for (auto it = items.rbegin(); it != items.rend(); ++it) {
        QwtPlotItem *item = *it;
        if (item->isVisible() && !item->title().isEmpty()) {
            QwtPlotShapeItem *shapeItem = static_cast<QwtPlotShapeItem *>(item);
            const QPointF p(coords[item->xAxis()], coords[item->yAxis()]);
            if (shapeItem->boundingRect().contains(p) && shapeItem->shape().contains(p)) {
                return shapeItem;
            }
        }
    }

    return nullptr;
}

