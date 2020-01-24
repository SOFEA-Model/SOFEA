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

#ifndef SCENARIOPAGES_H
#define SCENARIOPAGES_H

#include <QWidget>

#include "Scenario.h"
#include "FluxProfileModel.h"
#include "ProjectionEditor.h"
#include "widgets/DoubleLineEdit.h"
#include "widgets/ListEditor.h"
#include "widgets/ReadOnlyLineEdit.h"
#include "widgets/StandardTableEditor.h"
#include "widgets/StandardTableView.h"

QT_BEGIN_NAMESPACE
class QAction;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QDateEdit;
class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QStandardItemModel;
class QToolButton;
QT_END_NAMESPACE

/****************************************************************************
** General
****************************************************************************/

class GeneralPage : public QWidget
{
public:
    GeneralPage(Scenario *s, QWidget *parent = nullptr);
    void init();
    void save();
    void load();

private:
    Scenario *scenario;

    QComboBox *cboFumigant;
    DoubleLineEdit *leDecayCoefficient;
    ListEditor *periodEditor;
};

/****************************************************************************
** Projection
****************************************************************************/

class ProjectionPage : public QWidget
{
public:
    ProjectionPage(Scenario *s, QWidget *parent = nullptr);
    void init();
    void save();
    void load();

private:
    Scenario *scenario;

    ProjectionEditor *editor;
};

/****************************************************************************
** Meteorological Data
****************************************************************************/

class MetDataPage : public QWidget
{
    Q_OBJECT

public:
    MetDataPage(Scenario *s, QWidget *parent = nullptr);
    void init();
    void save();
    void load();

private slots:
    void browseMetDataFile();
    void showInfoDialog();
    void showDeclinationCalc();
    void update();

private:
    Scenario *scenario;

    QLineEdit *leSurfaceDataFile;        // SURFFILE
    QToolButton *btnSurfaceDataFile;
    QLineEdit *leUpperAirDataFile;       // PROFFILE
    QToolButton *btnUpperAirDataFile;
    QDoubleSpinBox *sbAnemometerHeight;  // PROFBASE/ANEMHGHT
    QDoubleSpinBox *sbWindRotation;      // WDROTATE
    QToolButton *btnDeclinationCalc;

    ReadOnlyLineEdit *leSurfaceStationId;
    ReadOnlyLineEdit *leUpperAirStationId;
    QListWidget *lwIntervals;
    QPushButton *btnDiagnostics;
    QPushButton *btnUpdate;
};

/****************************************************************************
** Flux Profiles
****************************************************************************/

class FluxProfilesPage : public QWidget
{
    Q_OBJECT

public:
    FluxProfilesPage(Scenario *s, QWidget *parent = nullptr);
    void init();
    void save();
    void load();

private slots:
    void editFluxProfile(const QModelIndex& index);

private:
    Scenario *scenario;

    FluxProfileModel *model;
    StandardTableView *table;
    StandardTableEditor *editor;
};

/****************************************************************************
** Dispersion Model
****************************************************************************/

class DispersionPage : public QWidget
{
    Q_OBJECT

public:
    DispersionPage(Scenario *s, QWidget *parent = nullptr);
    void init();
    void save();
    void load();

private slots:
    void resetState();

private:
    Scenario *scenario;

    // AERMOD
    QCheckBox *chkFlat;
    QCheckBox *chkFastArea;
    QCheckBox *chkDryDeposition;
    QCheckBox *chkDryDplt;
    QCheckBox *chkAreaDplt;
    QCheckBox *chkGDVelocity;
    QDoubleSpinBox *sbGDVelocity;
    QCheckBox *chkWetDeposition;
    QCheckBox *chkWetDplt;
    QCheckBox *chkLowWind;
    QLabel *lblSVmin;
    QLabel *lblWSmin;
    QLabel *lblFRANmax;
    QDoubleSpinBox *sbSVmin;
    QDoubleSpinBox *sbWSmin;
    QDoubleSpinBox *sbFRANmax;
};

#endif SCENARIOPAGES_H
