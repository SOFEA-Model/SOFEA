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
#include <qwt_plot_shapeitem.h>
#include <qwt_point_data.h>
#include <qwt_series_data.h>

#include "PlotCurveTracker.h"
#include "PlotItemTitleTracker.h"

class StandardPlot : public QwtPlot
{
    Q_OBJECT

public:
    explicit StandardPlot(QWidget *parent = nullptr);
    ~StandardPlot();

    void setXAxisTitle(const QString&);
    void setYAxisTitle(const QString&);
    void setXAxisScale(double min, double max, double step);
    void setYAxisScale(double min, double max, double step);
    void clear();
    void setPanZoomMode(bool);
    void setCurveTracker(bool);
    void setItemTitleTracker(bool);
    void setRescaler(bool);
    void addPoint(const QPointF&, const int size = 5, const QBrush& brush = QBrush());
    void addPoints(const QPolygonF&, const int size = 5, const QBrush& brush = QBrush());
    void addRing(const QPolygonF&, const QPen& pen = QPen(), const QBrush& brush = QBrush(), const QString& title = QString());
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
    PlotCurveTracker *m_curveTracker = nullptr;
    PlotItemTitleTracker *m_titleTracker = nullptr;
};

#endif // STANDARDPLOT_H
