#ifndef PLOTCURVETRACKER_H
#define PLOTCURVETRACKER_H

#include <qwt_plot_picker.h>

class QwtPlotCurve;

class PlotCurveTracker : public QwtPlotPicker
{
public:
    PlotCurveTracker(QWidget *canvas);

protected:
    virtual QRect trackerRect(const QFont &) const override;
    virtual QwtText trackerTextF(const QPointF &) const override;

private:
    QString curveInfoAt(const QwtPlotCurve *, const QPointF &) const;
    QLineF curveLineAt(const QwtPlotCurve *, double x) const;
};

#endif // PLOTCURVETRACKER_H
