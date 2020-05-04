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

#pragma once

#include <QWidget>
#include <QModelIndexList>

class AreaSourceGeometryEditor;
class AreaCircSourceGeometryEditor;
class AreaPolySourceGeometryEditor;
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

class SourceGeometryEditor : public QWidget
{
public:
    explicit SourceGeometryEditor(QWidget *parent = nullptr);
    void setModel(QAbstractItemModel *model);
    void setIndexes(const QModelIndexList& indexes);
    void setEditorIndex(int index);
    void setStatusText(const QString& text);

private:
    QLabel *statusLabel;
    AreaSourceGeometryEditor *areaEditor;
    AreaCircSourceGeometryEditor *areaCircEditor;
    AreaPolySourceGeometryEditor *areaPolyEditor;
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
    QStackedLayout *stack;
};

//-----------------------------------------------------------------------------
// AreaSourceGeometryEditor
//-----------------------------------------------------------------------------

class AreaSourceGeometryEditor : public QWidget
{
friend class SourceGeometryEditor;

public:
    explicit AreaSourceGeometryEditor(QWidget *parent = nullptr);

private:
    QDoubleSpinBox *sbXCoord;
    QDoubleSpinBox *sbYCoord;
    QDoubleSpinBox *sbXInit;
    QDoubleSpinBox *sbYInit;
    QDoubleSpinBox *sbAngle;
};

//-----------------------------------------------------------------------------
// AreaCircSourceGeometryEditor
//-----------------------------------------------------------------------------

class AreaCircSourceGeometryEditor : public QWidget
{
friend class SourceGeometryEditor;

public:
    explicit AreaCircSourceGeometryEditor(QWidget *parent = nullptr);

private:
    QDoubleSpinBox *sbXCoord;
    QDoubleSpinBox *sbYCoord;
    QDoubleSpinBox *sbRadius;
    QSpinBox *sbVertexCount;
};

//-----------------------------------------------------------------------------
// AreaCircSourceGeometryEditor
//-----------------------------------------------------------------------------

class AreaPolySourceGeometryEditor : public QWidget
{
friend class SourceGeometryEditor;

public:
    explicit AreaPolySourceGeometryEditor(QWidget *parent = nullptr);

private:
    VertexEditorDelegate *vertexEditorDelegate;
    VertexEditor *vertexEditor;
};
