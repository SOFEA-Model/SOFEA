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

#ifndef SOURCEGROUPPAGES_H
#define SOURCEGROUPPAGES_H

#include <QWidget>

#include "core/Scenario.h"
#include "core/SourceGroup.h"

class BufferZoneModel;
class BufferZoneEditor;
class FluxProfileModel;
class MonteCarloLineEdit;
class MonteCarloDateTimeEdit;
class SamplingDistributionEditor;
class StandardTableView;
class StatusLabel;

QT_BEGIN_NAMESPACE
class QAction;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QDataWidgetMapper;
class QDateEdit;
class QDoubleSpinBox;
class QGroupBox;
class QItemSelection;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QStackedWidget;
class QStandardItemModel;
QT_END_NAMESPACE

/****************************************************************************
** Application
****************************************************************************/

class ApplicationPage : public QWidget
{
    Q_OBJECT
public:
    ApplicationPage(SourceGroup *sg, QWidget *parent = nullptr);
    void init();
    void save();
    void load();

    MonteCarloDateTimeEdit *mcAppStart;
    MonteCarloLineEdit *mcAppRate;
    MonteCarloLineEdit *mcIncorpDepth;

private:
    SourceGroup *sgPtr;

    static const QMap<SourceGroup::AppMethod, QString> appMethodMap;

    QComboBox *cboAppMethod;
    QDoubleSpinBox *sbAppFactor;
    QButtonGroup *bgCalcMode;
    QRadioButton *radioProspective;
    QRadioButton *radioValidation;
    QGroupBox *gbMonteCarlo;
};

/****************************************************************************
** Deposition
****************************************************************************/

class DepositionPage : public QWidget
{
public:
    DepositionPage(SourceGroup *sg, QWidget *parent = nullptr);
    void init();
    void warnDepoNotEnabled();
    void warnUserVelocity();
    void load();
    void save();

    MonteCarloLineEdit *mcAirDiffusion;
    MonteCarloLineEdit *mcWaterDiffusion;
    MonteCarloLineEdit *mcCuticularResistance;
    MonteCarloLineEdit *mcHenryConstant;

private:
    StatusLabel *lblDepoNotEnabled;
    StatusLabel *lblDepoUserVelocity;
    SourceGroup *sgPtr;
};

/****************************************************************************
** Flux Profile
****************************************************************************/

class FluxProfilePage : public QWidget
{
    Q_OBJECT
public:
    FluxProfilePage(Scenario *s, SourceGroup *sg, QWidget *parent = nullptr);
    void init();
    void save();
    void load();

private:
    Scenario *sPtr;
    SourceGroup *sgPtr;

    FluxProfileModel *model;
    SamplingDistributionEditor *editor;
    StatusLabel *lblNoFluxProfile;
};

/****************************************************************************
** Buffer Zone
****************************************************************************/

class BufferZonePage : public QWidget
{
    Q_OBJECT
public:
    BufferZonePage(SourceGroup *sg, QWidget *parent = nullptr);
    void init();
    void save();
    void load();

private:
    SourceGroup *sgPtr;

    BufferZoneModel *model;
    BufferZoneEditor *editor;
};

#endif // SOURCEGROUPPAGES_H
