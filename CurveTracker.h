#ifndef CURVETRACKER_H
#define CURVETRACKER_H

#include <qwt_plot_picker.h>

class QwtPlotCurve;

class CurveTracker : public QwtPlotPicker
{
public:
    CurveTracker(QWidget *);

protected:
    virtual QRect trackerRect(const QFont &) const;
    virtual QwtText trackerTextF(const QPointF &) const;

private:
    QString curveInfoAt(const QwtPlotCurve *, const QPointF &) const;
    QLineF curveLineAt(const QwtPlotCurve *, double x) const;
};

#endif // CURVETRACKER_H
