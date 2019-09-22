#include "PlotCurveTracker.h"

#include <QApplication>

#include <qwt_picker_machine.h>
#include <qwt_series_data.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>

struct compareX
{
    inline bool operator()(const double x, const QPointF &pos) const
    {
        return (x < pos.x());
    }
};

PlotCurveTracker::PlotCurveTracker(QWidget *canvas) : QwtPlotPicker(canvas)
{
    setTrackerMode(QwtPlotPicker::ActiveOnly);
    setStateMachine(new QwtPickerTrackerMachine());
    setRubberBand(VLineRubberBand);
    QColor color = QApplication::palette().color(QPalette::WindowText);
    setRubberBandPen(QPen(color, 1, Qt::SolidLine));
}

QRect PlotCurveTracker::trackerRect(const QFont &font) const
{
    QRect r = QwtPlotPicker::trackerRect(font);
    
    // align r to the first curve
    const QwtPlotItemList curves = plot()->itemList(QwtPlotItem::Rtti_PlotCurve);
    if (curves.size() > 0)
    {
        QPointF pos = invTransform(trackerPosition());

        const QwtPlotCurve* curve = static_cast<const QwtPlotCurve *>(curves[0]);
        const QLineF line = curveLineAt(curve, pos.x());
        if (!line.isNull()) {
            const double curveY = line.pointAt((pos.x() - line.p1().x()) / line.dx()).y();
            pos.setY(curveY);
            pos = transform(pos);

            // ensure r is inside margin
            int delta = qMax(r.height() - (int)pos.y(), 0);
            r.moveBottom(pos.y() + delta);
        }
    }

    return r;
}

QwtText PlotCurveTracker::trackerTextF(const QPointF &pos) const
{
    // call curveInfoAt for all curves and return text
    QwtText trackerText;
    QFont trackerFont = QApplication::font();
    trackerFont.setPointSize(8);
    trackerText.setFont(trackerFont);
    QColor textColor = QApplication::palette().color(QPalette::WindowText);
    trackerText.setColor(textColor);

    QString info;

    const QwtPlotItemList curves = plot()->itemList(QwtPlotItem::Rtti_PlotCurve);
    for (int i = 0; i < curves.size(); i++)
    {
        const QwtPlotCurve* curve = static_cast<const QwtPlotCurve *>(curves[i]);
        const QString curveInfo = curveInfoAt(curve, pos);

        if (!curveInfo.isEmpty()) {
            if (!info.isEmpty())
                info += "<br>";

            info += curveInfo;
        }
    }

    trackerText.setText(info);
    return trackerText;
}

QString PlotCurveTracker::curveInfoAt(const QwtPlotCurve *curve, const QPointF &pos) const
{
    // get y-coordinate QString at the current curve
    const QLineF line = curveLineAt(curve, pos.x());
    if (line.isNull())
        return QString();

    double y = line.pointAt((pos.x() - line.p1().x()) / line.dx()).y();

    return QString::number(y);
}

QLineF PlotCurveTracker::curveLineAt(const QwtPlotCurve *curve, double x) const
{
    QLineF line;

    if (curve->dataSize() >= 2)
    {
        const QRectF br = curve->boundingRect();

        if ((br.width() > 0) && (x >= br.left()) && (x <= br.right()))
        {
            int index = qwtUpperSampleIndex<QPointF>(*curve->data(), x, compareX());

            int dataSize = static_cast<int>(curve->dataSize());
            if (index == -1 && x == curve->sample(dataSize - 1).x()) {
                // the last sample is excluded from qwtUpperSampleIndex
                index = dataSize - 1;
            }

            if (index > 0) {
                line.setP1(curve->sample(index - 1));
                line.setP2(curve->sample(index));
            }
        }
    }
    
    return line;
}
