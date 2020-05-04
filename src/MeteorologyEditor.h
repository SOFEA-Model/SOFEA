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

#ifndef METEOROLOGYEDITOR_H
#define METEOROLOGYEDITOR_H

#include <QWidget>

class ButtonLineEdit;
class MeteorologyModel;
class MeteorologyTableView;
class PathEdit;
class StandardTableEditor;
class StatusLabel;

QT_BEGIN_NAMESPACE
class QAction;
class QDataWidgetMapper;
class QDoubleSpinBox;
class QItemSelection;
class QLabel;
class QPushButton;
class QToolButton;
QT_END_NAMESPACE

class MeteorologyEditor : public QWidget
{
    Q_OBJECT

public:
    MeteorologyEditor(MeteorologyModel *m, QWidget *parent = nullptr);

private slots:
    void onSelectionChanged(const QItemSelection&, const QItemSelection&);
    void onAddClicked();
    void onWindRoseClicked();
    void onDeclinationCalcClicked();
    void onCurrentPathChanged(const QString& path);

private:
    MeteorologyModel *model;
    MeteorologyTableView *table;
    StandardTableEditor *tableEditor;
    StatusLabel *lblInfoTip;
    QPushButton *btnWindRose;
    PathEdit *leSurfaceFile;
    PathEdit *leUpperAirFile;
    QDoubleSpinBox *sbTerrainElevation;
    QDoubleSpinBox *sbAnemometerHeight;
    QDoubleSpinBox *sbWindRotation;
    QToolButton *btnDeclinationCalc;
    QPushButton *btnUpdate;
    QDataWidgetMapper *mapper;
};

#endif // METEOROLOGYEDITOR_H
