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

#ifndef GENERICDISTRIBUTIONDIALOG_H
#define GENERICDISTRIBUTIONDIALOG_H

#include <QDialog>

#include "GenericDistribution.h"
#include "StandardPlot.h"

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QDoubleSpinBox;
class QLabel;
class QStandardItemModel;
class QTreeView;
QT_END_NAMESPACE

class GenericDistributionDialog : public QDialog
{
    Q_OBJECT

public:
    GenericDistributionDialog(const GenericDistribution &d, QWidget *parent = nullptr);
    GenericDistribution getDistribution() const;

private:
    void initializeModel();
    void initializeConnections();
    void resetControls();
    void setDistribution(GenericDistribution::DistributionID id);
    void updatePlot();

private slots:
    void onValueChanged(double);

private:
    GenericDistribution currentDist;
    GenericDistribution::DistributionID currentID;

    QStandardItemModel *distributionModel;
    QTreeView *distributionTree;

    QLabel *label1;
    QLabel *label2;
    QLabel *label3;

    QDoubleSpinBox *sb1;
    QDoubleSpinBox *sb2;
    QDoubleSpinBox *sb3;

    StandardPlot *pdfPlot;
    QwtPointSeriesData *pdfPlotCurveData;
    QwtPlotCurve *pdfPlotCurve;

    QDialogButtonBox *buttonBox;
};

#endif // GENERICDISTRIBUTIONDIALOG_H
