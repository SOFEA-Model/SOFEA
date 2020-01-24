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

#ifndef FLUXPROFILEDIALOG_H
#define FLUXPROFILEDIALOG_H

#include <memory>

#include <QDialog>
#include <QMap>

#include "FluxProfile.h"
#include "widgets/StandardTableEditor.h"
#include "widgets/StandardTableView.h"

QT_BEGIN_NAMESPACE
class QButtonGroup;
class QComboBox;
class QDateEdit;
class QDateTimeEdit;
class QDialogButtonBox;
class QDoubleSpinBox;
class QPushButton;
class QRadioButton;
class QStackedWidget;
class QStandardItemModel;
QT_END_NAMESPACE

class FluxProfileDialog : public QDialog
{
    Q_OBJECT

public:
    FluxProfileDialog(std::shared_ptr<FluxProfile> fp, QWidget *parent = nullptr);

private:
    void init();
    void save();
    void load();
    FluxProfile currentProfile();

private slots:
    void onRowsInserted(const QModelIndex& parent, int first, int last);
    void importFluxProfile(const QString& filename);
    void calcPhase();
    void plotFluxProfile();
    void plotTemporalScaling();
    void plotDepthScaling();

public slots:
    void accept() override;
    
private:
    std::weak_ptr<FluxProfile> wptr;

    static const QMap<FluxProfile::TSMethod, QString> tsMethodMap;
    static const QMap<FluxProfile::DSMethod, QString> dsMethodMap;

    QDialogButtonBox *buttonBox;
    QPushButton *btnPlot;

    // Reference
    QDateTimeEdit *deRefDate;
    QDoubleSpinBox *sbRefAppRate;
    //QButtonGroup *bgFluxMode;
    //StatusLabel *lblConstantFluxInfo;
    //QRadioButton *radioConstantFlux;
    //QRadioButton *radioVariableFlux;
    QDoubleSpinBox *sbRefDepth;
    QDoubleSpinBox *sbRefVL;
    QDoubleSpinBox *sbMaxVL;
    QStandardItemModel *refModel;
    StandardTableView *refTable;
    StandardTableEditor *refEditor;

    // Temporal Scaling
    QComboBox *cboTemporalScaling;
    QStackedWidget *temporalStack;

    // Temporal Scaling - CDPR
    QDateEdit *deStartDate;
    QDateEdit *deEndDate;
    QDoubleSpinBox *sbScaleFactor;

    // Temporal Scaling - Sinusoidal
    QDoubleSpinBox *sbAmplitude;
    QDoubleSpinBox *sbCenterAmplitude;
    QDoubleSpinBox *sbPhase;
    QDoubleSpinBox *sbWavelength;
    QPushButton *btnCalcPhase;
    QPushButton *btnPlotTS;

    // Incorporation Depth Scaling
    QComboBox *cboDepthScaling;
    QPushButton *btnPlotDS;
};

#endif // FLUXPROFILEDIALOG_H
