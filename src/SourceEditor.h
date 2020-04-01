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

#ifndef SOURCEEDITOR_H
#define SOURCEEDITOR_H

#include <vector>

#include <QWidget>

#include "core/SourceGroup.h"

class AreaSourceEditor;
class AreaCircSourceEditor;
class DoubleLineEdit;
class VertexEditor;
class VertexEditorDelegate;

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
class QDataWidgetMapper;
class QDateTimeEdit;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPolygonF;
class QPushButton;
class QSpinBox;
class QStackedLayout;
QT_END_NAMESPACE

class SourceEditor : public QWidget
{
    Q_OBJECT

public:
    SourceEditor(QWidget *parent = nullptr);
    void setModel(QAbstractItemModel *model);
    void setCurrentModelIndex(const QModelIndex& index);
    void setEditorIndex(int index);
    void setStatusText(const QString& text);

private:
    QLabel *statusLabel;
    AreaSourceEditor *areaEditor;
    AreaCircSourceEditor *areaCircEditor;
    VertexEditorDelegate *areaPolyEditorDelegate;
    VertexEditor *areaPolyEditor;
    QPushButton *btnUpdate;

    //QDateTimeEdit *deAppStart;
    //DoubleLineEdit *leAppRate;
    //DoubleLineEdit *leIncorpDepth;

    //DoubleLineEdit *leAirDiffusion;
    //DoubleLineEdit *leWaterDiffusion;
    //DoubleLineEdit *leCuticularResistance;
    //DoubleLineEdit *leHenryConstant;

    QDataWidgetMapper *mapper;
    QAbstractItemModel *sourceModel = nullptr;
    QStackedLayout *geometryStack;
};

//-----------------------------------------------------------------------------
// AreaSourceEditor
//-----------------------------------------------------------------------------

class AreaSourceEditor : public QWidget
{
    Q_OBJECT

friend class SourceEditor;

public:
    AreaSourceEditor(QWidget *parent = nullptr);

private:
    QDoubleSpinBox *sbXCoord;
    QDoubleSpinBox *sbYCoord;
    QDoubleSpinBox *sbXInit;
    QDoubleSpinBox *sbYInit;
    QDoubleSpinBox *sbAngle;
};

//-----------------------------------------------------------------------------
// AreaCircSourceEditor
//-----------------------------------------------------------------------------

class AreaCircSourceEditor : public QWidget
{
    Q_OBJECT

friend class SourceEditor;

public:
    AreaCircSourceEditor(QWidget *parent = nullptr);

private:
    QDoubleSpinBox *sbXCoord;
    QDoubleSpinBox *sbYCoord;
    QDoubleSpinBox *sbRadius;
    QSpinBox *sbVertexCount;
};

#endif // SOURCEEDITOR_H
