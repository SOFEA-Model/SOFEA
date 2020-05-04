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

#include <QAbstractItemModel>
#include <QBoxLayout>
#include <QDataWidgetMapper>
#include <QDateTimeEdit>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPolygonF>
#include <QPushButton>
#include <QStackedLayout>

#include "SourceGeometryEditor.h"
#include "core/Common.h"
#include "models/SourceModel.h"
#include "widgets/VertexEditor.h"
#include "delegates/VertexEditorDelegate.h"

SourceGeometryEditor::SourceGeometryEditor(QWidget *parent) : QWidget(parent)
{
    statusLabel = new QLabel;
    statusLabel->setText("No sources selected.");
    statusLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    areaEditor = new AreaSourceGeometryEditor;
    areaCircEditor = new AreaCircSourceGeometryEditor;
    areaPolyEditor = new AreaPolySourceGeometryEditor;

    btnUpdate = new QPushButton(tr("Update"));
    QSizePolicy btnUpdateSizePolicy = btnUpdate->sizePolicy();
    btnUpdateSizePolicy.setRetainSizeWhenHidden(true);
    btnUpdate->setSizePolicy(btnUpdateSizePolicy);
    btnUpdate->setAutoDefault(true);

    //deAppStart = new QDateTimeEdit;
    //deAppStart->setTimeSpec(Qt::UTC);
    //deAppStart->setDisplayFormat("yyyy-MM-dd HH:mm");

    //sbAppRate = new DoubleLineEdit;
    //sbAppRate->setRange(0, 10000000);
    //sbAppRate->setDecimals(2);

    //sbIncorpDepth = new DoubleLineEdit;
    //sbIncorpDepth->setRange(0, 100);
    //sbIncorpDepth->setDecimals(2);

    //leAirDiffusion = new DoubleLineEdit;
    //leAirDiffusion->setRange(0.000001, 0.5);
    //leAirDiffusion->setDecimals(8);

    //leWaterDiffusion = new DoubleLineEdit;
    //leWaterDiffusion->setRange(0.000001, 0.5);
    //leWaterDiffusion->setDecimals(8);

    //leCuticularResistance = new DoubleLineEdit;
    //leCuticularResistance->setRange(0, 1000000);
    //leCuticularResistance->setDecimals(8);

    //leHenryConstant = new DoubleLineEdit;
    //leHenryConstant->setRange(0, 10000);
    //leHenryConstant->setDecimals(8);

    // Advanced
    // sbEmissionRate = new DoubleLineEdit;
    // sbReleaseHeight = new DoubleLineEdit;
    // sbSigmaZ0 = new DoubleLineEdit;

    mapper = new QDataWidgetMapper(this);
    mapper->setOrientation(Qt::Horizontal);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);

    // Layout
    stack = new QStackedLayout;
    stack->setContentsMargins(0, 0, 0, 0);
    stack->addWidget(statusLabel);
    stack->addWidget(areaEditor);
    stack->addWidget(areaCircEditor);
    stack->addWidget(areaPolyEditor);

    QHBoxLayout *buttonBox = new QHBoxLayout;
    buttonBox->setContentsMargins(0, 0, 0, 0);
    buttonBox->addStretch(1);
    buttonBox->addWidget(btnUpdate);

    QVBoxLayout *geometryLayout = new QVBoxLayout;
    geometryLayout->setContentsMargins(24, 16, 24, 16);
    geometryLayout->addLayout(stack);
    geometryLayout->addSpacing(5);
    geometryLayout->addLayout(buttonBox);

    //QGridLayout *applicationLayout = new QGridLayout;
    //applicationLayout->setColumnMinimumWidth(0, 225);
    //applicationLayout->setColumnStretch(0, 1);
    //applicationLayout->setColumnStretch(1, 2);
    //applicationLayout->setContentsMargins(24, 16, 24, 16);
    //applicationLayout->addWidget(deAppStart, 0, 1);
    //applicationLayout->addWidget(sbAppRate, 0, 1);
    //applicationLayout->addWidget(sbIncorpDepth, 0, 1);

    //QGridLayout *depositionLayout = new QGridLayout;
    //depositionLayout->setColumnMinimumWidth(0, 225);
    //depositionLayout->setColumnStretch(0, 1);
    //depositionLayout->setColumnStretch(1, 2);
    //depositionLayout->setContentsMargins(24, 16, 24, 16);

    QGroupBox *gbGeometry = new QGroupBox(tr("Geometry"));
    gbGeometry->setLayout(geometryLayout);
    gbGeometry->setFlat(true);

    //QGroupBox *gbApplication = new QGroupBox(tr("Application"));
    //gbApplication->setLayout(applicationLayout);
    //gbApplication->setFlat(true);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(gbGeometry);
    //mainLayout->addWidget(gbApplication);
    mainLayout->addStretch(1);

    stack->setCurrentIndex(0);

    connect(btnUpdate, &QPushButton::pressed, mapper, &QDataWidgetMapper::submit);

    setLayout(mainLayout);
    btnUpdate->setVisible(false);
}

void SourceGeometryEditor::setModel(QAbstractItemModel *model)
{
    if (model == nullptr)
        return;

    sourceModel = model;
    mapper->setModel(model);
}

void SourceGeometryEditor::setIndexes(const QModelIndexList& indexes)
{
    if (!sourceModel) {
        setStatusText("Invalid source data.");
        return;
    }
    if (indexes.empty()) {
        setStatusText("No sources selected.");
        return;
    }
    else if (indexes.size() > 1) {
        setStatusText("Multiple sources selected.");
        return;
    }

    QModelIndex index = indexes.front();

    if (!index.isValid() || index.model() == nullptr) {
        setStatusText("Invalid source selected.");
        return;
    }

    QVariant sourceTypeVar = index.model()->headerData(index.row(), Qt::Vertical, Qt::UserRole);
    if (!sourceTypeVar.canConvert<SourceType>()) {
        setStatusText("Invalid source type.");
        return;
    }

    SourceType sourceType = qvariant_cast<SourceType>(sourceTypeVar);
    switch (sourceType) {
    case SourceType::AREA:
        if (stack->currentWidget() != areaEditor) {
            mapper->clearMapping();
            mapper->addMapping(areaEditor->sbXCoord, SourceModel::Column::X);
            mapper->addMapping(areaEditor->sbYCoord, SourceModel::Column::Y);
            mapper->addMapping(areaEditor->sbXInit, SourceModel::Column::XInit);
            mapper->addMapping(areaEditor->sbYInit, SourceModel::Column::YInit);
            mapper->addMapping(areaEditor->sbAngle, SourceModel::Column::Angle);
            mapper->toFirst();
        }
        stack->setCurrentWidget(areaEditor);
        mapper->setCurrentIndex(index.row());
        btnUpdate->setVisible(true);
        break;
    case SourceType::AREACIRC:
        if (stack->currentWidget() != areaCircEditor) {
            mapper->clearMapping();
            mapper->addMapping(areaCircEditor->sbXCoord, SourceModel::Column::X);
            mapper->addMapping(areaCircEditor->sbYCoord, SourceModel::Column::Y);
            mapper->addMapping(areaCircEditor->sbRadius, SourceModel::Column::Radius);
            mapper->addMapping(areaCircEditor->sbVertexCount, SourceModel::Column::VertexCount);
            mapper->toFirst();
        }
        stack->setCurrentWidget(areaCircEditor);
        mapper->setCurrentIndex(index.row());
        btnUpdate->setVisible(true);
        break;
    case SourceType::AREAPOLY:
        if (stack->currentWidget() != areaPolyEditor) {
            mapper->clearMapping();
            mapper->addMapping(areaPolyEditor->vertexEditor, 0);
            mapper->setItemDelegate(areaPolyEditor->vertexEditorDelegate);
            mapper->toFirst();
        }
        stack->setCurrentWidget(areaPolyEditor);
        mapper->setCurrentIndex(index.row());
        btnUpdate->setVisible(true);
        break;
    case SourceType::POINT:
        setStatusText("Unsupported source type (POINT).");
        break;
    case SourceType::POINTCAP:
        setStatusText("Unsupported source type (POINTCAP).");
        break;
    case SourceType::POINTHOR:
        setStatusText("Unsupported source type (POINTHOR).");
        break;
    case SourceType::VOLUME:
        setStatusText("Unsupported source type (VOLUME).");
        break;
    case SourceType::OPENPIT:
        setStatusText("Unsupported source type (OPENPIT).");
        break;
    case SourceType::LINE:
        setStatusText("Unsupported source type (LINE).");
        break;
    case SourceType::BUOYLINE:
        setStatusText("Unsupported source type (BUOYLINE).");
        break;
    case SourceType::RLINE:
        setStatusText("Unsupported source type (RLINE).");
        break;
    case SourceType::RLINEXT:
        setStatusText("Unsupported source type (RLINEXT).");
        break;
    default:
        setStatusText("Unsupported source type.");
        break;
    }
}

void SourceGeometryEditor::setStatusText(const QString& text)
{
    statusLabel->setText(text);
    btnUpdate->setVisible(false);
    mapper->clearMapping();
    stack->setCurrentIndex(0);
}

//-----------------------------------------------------------------------------
// AreaSourceGeometryEditor
//-----------------------------------------------------------------------------

AreaSourceGeometryEditor::AreaSourceGeometryEditor(QWidget *parent) : QWidget(parent)
{
    using namespace sofea::constants;

    sbXCoord = new QDoubleSpinBox();
    sbXCoord->setRange(MIN_X_COORDINATE, MAX_X_COORDINATE);
    sbXCoord->setDecimals(X_COORDINATE_PRECISION);
    sbXCoord->setButtonSymbols(QAbstractSpinBox::NoButtons);

    sbYCoord = new QDoubleSpinBox();
    sbYCoord->setRange(MIN_Y_COORDINATE, MAX_Y_COORDINATE);
    sbYCoord->setDecimals(Y_COORDINATE_PRECISION);
    sbYCoord->setButtonSymbols(QAbstractSpinBox::NoButtons);

    sbXInit = new QDoubleSpinBox();
    sbXInit->setRange(MIN_X_DIMENSION, MAX_X_DIMENSION);
    sbXInit->setDecimals(Y_DIMENSION_PRECISION);
    sbXInit->setButtonSymbols(QAbstractSpinBox::NoButtons);

    sbYInit = new QDoubleSpinBox();
    sbYInit->setRange(MIN_Y_DIMENSION, MAX_Y_DIMENSION);
    sbYInit->setDecimals(Y_DIMENSION_PRECISION);
    sbYInit->setButtonSymbols(QAbstractSpinBox::NoButtons);

    sbAngle = new QDoubleSpinBox;
    sbAngle->setMinimum(MIN_ROTATION_ANGLE);
    sbAngle->setMaximum(MAX_ROTATION_ANGLE);
    sbAngle->setSingleStep(1);
    sbAngle->setDecimals(ROTATION_ANGLE_PRECISION);
    sbAngle->setWrapping(true);

    // Layout
    QGridLayout *layout1 = new QGridLayout;
    layout1->setContentsMargins(0, 0, 0, 0);
    layout1->setColumnMinimumWidth(0, 120);
    layout1->addWidget(new QLabel(tr("Coordinates:")), 0, 0);
    layout1->addWidget(sbXCoord, 0, 1);
    layout1->addWidget(sbYCoord, 0, 2);
    layout1->addWidget(new QLabel(tr("Dimensions:")), 1, 0);
    layout1->addWidget(sbXInit, 1, 1);
    layout1->addWidget(sbYInit, 1, 2);
    layout1->addWidget(new QLabel(QLatin1String("Angle (\u00b0CW):")), 2, 0);
    layout1->addWidget(sbAngle, 2, 1, 1, 2);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(layout1);
    mainLayout->addStretch(1);

    setLayout(mainLayout);
}

//-----------------------------------------------------------------------------
// AreaCircSourceGeometryEditor
//-----------------------------------------------------------------------------

AreaCircSourceGeometryEditor::AreaCircSourceGeometryEditor(QWidget *parent) : QWidget(parent)
{
    using namespace sofea::constants;

    sbXCoord = new QDoubleSpinBox();
    sbXCoord->setRange(MIN_X_COORDINATE, MAX_X_COORDINATE);
    sbXCoord->setDecimals(X_COORDINATE_PRECISION);
    sbXCoord->setButtonSymbols(QAbstractSpinBox::NoButtons);

    sbYCoord = new QDoubleSpinBox();
    sbYCoord->setRange(MIN_Y_COORDINATE, MAX_Y_COORDINATE);
    sbYCoord->setDecimals(Y_COORDINATE_PRECISION);
    sbYCoord->setButtonSymbols(QAbstractSpinBox::NoButtons);

    sbRadius = new QDoubleSpinBox();
    sbRadius->setRange(MIN_RADIUS, MAX_RADIUS);
    sbRadius->setDecimals(RADIUS_PRECISION);
    sbRadius->setButtonSymbols(QAbstractSpinBox::NoButtons);

    sbVertexCount = new QSpinBox;
    sbVertexCount->setRange(3, 100);

    // Layout
    QGridLayout *layout1 = new QGridLayout;
    layout1->setContentsMargins(0, 0, 0, 0);
    layout1->setColumnMinimumWidth(0, 120);
    layout1->addWidget(new QLabel(tr("Coordinates:")), 0, 0);
    layout1->addWidget(sbXCoord, 0, 1);
    layout1->addWidget(sbYCoord, 0, 2);
    layout1->addWidget(new QLabel(tr("Radius:")), 1, 0);
    layout1->addWidget(sbRadius, 1, 1, 1, 2);
    layout1->addWidget(new QLabel(tr("Vertex count:")), 2, 0);
    layout1->addWidget(sbVertexCount, 2, 1, 1, 2);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(layout1);
    mainLayout->addStretch(1);

    setLayout(mainLayout);
}

//-----------------------------------------------------------------------------
// AreaPolySourceGeometryEditor
//-----------------------------------------------------------------------------

AreaPolySourceGeometryEditor::AreaPolySourceGeometryEditor(QWidget *parent) : QWidget(parent)
{
    vertexEditorDelegate = new VertexEditorDelegate(this);
    vertexEditor = qobject_cast<VertexEditor *>(
        vertexEditorDelegate->createEditor(nullptr, QStyleOptionViewItem(), QModelIndex()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(vertexEditor);
    mainLayout->addStretch(1);

    setLayout(mainLayout);
}
