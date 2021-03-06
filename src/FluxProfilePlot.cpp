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

#include <QBoxLayout>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QPointF>
#include <QVector>

#include "FluxProfilePlot.h"

FluxProfilePlot::FluxProfilePlot(FluxProfile fp, QWidget *parent)
    : QWidget(parent), fluxProfile(fp)
{
    setMinimumWidth(1000);
    setMinimumHeight(400);

    deAppStart = new QDateTimeEdit;
    deAppStart->setTimeSpec(Qt::UTC);
    deAppStart->setDisplayFormat("yyyy-MM-dd HH:mm");
    deAppStart->setDateTime(QDateTime(QDate(2000, 1, 1), QTime(0, 0, 0), Qt::UTC));

    sbAppRate = new QDoubleSpinBox;
    sbAppRate->setRange(0, 10000000);
    sbAppRate->setDecimals(2);

    sbIncorpDepth = new QDoubleSpinBox;
    sbIncorpDepth->setRange(2.54, 100);
    sbIncorpDepth->setDecimals(2);

    plot = new StandardPlot;
    plot->setCurveTracker(true);

    curve = new QwtPlotCurve;
    curve->setStyle(QwtPlotCurve::Steps);
    curve->setCurveAttribute(QwtPlotCurve::Inverted);
    curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    QPen curvePen = curve->pen();
    curvePen.setColor(QWidget::palette().text().color());
    curve->setPen(curvePen);
    curve->attach(plot);

    QwtDateScaleDraw *scaleDraw = new QwtDateScaleDraw(Qt::UTC);
    scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    scaleDraw->setDateFormat(QwtDate::Day, QString("yyyy-MM-dd"));
    plot->setAxisScaleDraw(QwtPlot::xBottom, scaleDraw);
    QwtDateScaleEngine *scaleEngine = new QwtDateScaleEngine(Qt::UTC);
    plot->setAxisScaleEngine(QwtPlot::xBottom, scaleEngine);

    // Layout
    QFormLayout *controlsLayout = new QFormLayout;
    controlsLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    controlsLayout->addRow("Application start time: ", deAppStart);
    controlsLayout->addRow("Application rate (kg/ha): ", sbAppRate);
    controlsLayout->addRow("Incorporation depth (cm):", sbIncorpDepth);

    QFrame *plotFrame = new QFrame;
    QVBoxLayout *plotLayout = new QVBoxLayout;
    plotLayout->addWidget(plot);
    plotFrame->setLayout(plotLayout);
    plotFrame->setFrameShape(QFrame::StyledPanel);
    plotFrame->setBackgroundRole(QPalette::Base);
    plotFrame->setAutoFillBackground(true);

    // Main Layout
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->addLayout(controlsLayout, 0);
    mainLayout->addWidget(plotFrame, 1);

    setLayout(mainLayout);
}

void FluxProfilePlot::setupConnections()
{
    connect(deAppStart, &QDateTimeEdit::dateTimeChanged,
        [=](const QDateTime &) { updatePlot(); });
    connect(sbAppRate, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](double) { updatePlot(); });
    connect(sbIncorpDepth, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [=](double) { updatePlot(); });
}

void FluxProfilePlot::setAppStart(const QDateTime& appStart)
{
    deAppStart->setDateTime(appStart);
}

void FluxProfilePlot::setAppRate(double appRate)
{
    sbAppRate->setValue(appRate);
}

void FluxProfilePlot::setIncorpDepth(double incorpDepth)
{
    sbIncorpDepth->setValue(incorpDepth);
}

void FluxProfilePlot::setControlsEnabled(bool enabled)
{
    deAppStart->setEnabled(enabled);
    sbAppRate->setEnabled(enabled);
    sbIncorpDepth->setEnabled(enabled);
}

void FluxProfilePlot::updatePlot()
{
    using GeneratedFlux = FluxProfile::GeneratedFlux;

    double appRate = sbAppRate->value();
    QDateTime appStart = deAppStart->dateTime();
    double incorpDepth = sbIncorpDepth->value();

    GeneratedFlux flux = fluxProfile.scaledFlux(appRate, appStart, incorpDepth);

    if (flux.empty())
        return;

    QVector<QPointF> series;
    series.reserve(static_cast<int>(flux.size()) + 1);

    for (const auto& xy : flux) {
        double x = QwtDate::toDouble(xy.first);
        double y = xy.second;
        QPointF p(x, y);
        series.push_back(p);
    }

    // Add the endpoint for the step function (+1 hour).
    QDateTime dtn = flux.back().first;
    dtn = dtn.addSecs(60 * 60);
    double xn = QwtDate::toDouble(dtn);
    double yn = flux.back().second;
    QPointF pn(xn, yn);
    series.push_back(pn);

    // Update the axis scale.
    double x0 = QwtDate::toDouble(appStart);
    double x1 = QwtDate::toDouble(dtn);
    plot->setAxisScale(QwtPlot::xBottom, x0, x1);

    // Update the curve.
    curve->setSamples(series);
    plot->replot();
}
