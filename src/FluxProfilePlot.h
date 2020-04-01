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

#ifndef FLUXPROFILEPLOT_H
#define FLUXPROFILEPLOT_H

#include <QWidget>

#include "core/FluxProfile.h"
#include "StandardPlot.h"

QT_BEGIN_NAMESPACE
class QDateTime;
class QDateTimeEdit;
class QDoubleSpinBox;
QT_END_NAMESPACE

class FluxProfilePlot : public QWidget
{
    Q_OBJECT

public:
    FluxProfilePlot(FluxProfile fp, QWidget *parent = nullptr);
    void setupConnections();
    void setAppStart(const QDateTime& appStart);
    void setAppRate(double appRate);
    void setIncorpDepth(double incorpDepth);
    void setControlsEnabled(bool enabled);

public slots:
    void updatePlot();

private:
    QDateTimeEdit *deAppStart;
    QDoubleSpinBox *sbAppRate;
    QDoubleSpinBox *sbIncorpDepth;
    StandardPlot *plot;
    QwtPlotCurve *curve;
    FluxProfile fluxProfile;
};

#endif // FLUXPROFILEPLOT_H
