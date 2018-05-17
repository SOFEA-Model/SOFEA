#ifndef STANDARDPLOT_H
#define STANDARDPLOT_H

#include <qwt_date.h>
#include <qwt_date_scale_draw.h>
#include <qwt_date_scale_engine.h>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_rescaler.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_point_data.h>
#include <qwt_series_data.h>

#include "CurveTracker.h"

class StandardPlot : public QwtPlot
{
    Q_OBJECT

public:
    explicit StandardPlot(QWidget *parent = nullptr);
    void setXAxisTitle(const QString&);
    void setYAxisTitle(const QString&);
    void setXAxisScale(double min, double max, double step);
    void setYAxisScale(double min, double max, double step);
    void clear();
    void setPanZoomMode(bool);
    void setCurveTracker(bool);
    void setRescaler(bool);
    void addPoint(QPointF const&, const int size = 5, QBrush const& brush = QBrush());
    void addPoints(QPolygonF const&, const int size = 5, QBrush const& brush = QBrush());
    void addRing(QPolygonF const&, QPen const& pen = QPen(), QBrush const& brush = QBrush());
    void addRefVLine(double x, double y, const QString& label);
    void autoScale();

private:
    void setAxisTitleInternal(QwtPlot::Axis axis, const QString& text);

    QwtPlotCanvas *m_canvas;
    QwtPlotGrid *m_grid;

    // Optional
    QwtPlotPanner *m_panner = nullptr;
    QwtPlotMagnifier *m_magnifier = nullptr;
    QwtPlotRescaler* m_rescaler = nullptr;
    CurveTracker *m_tracker = nullptr;
};

#endif // STANDARDPLOT_H
