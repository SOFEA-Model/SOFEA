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
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPolygonF>
#include <QPushButton>
#include <QStackedLayout>

#include "SourceEditor.h"
#include "SourceModel.h"

#include <ctk/ctkCollapsibleGroupBox.h>

SourceEditor::SourceEditor(QWidget *parent) : QWidget(parent)
{
    statusLabel = new QLabel;
    statusLabel->setText("No sources selected.");
    statusLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBaseline);

    areaPolyEditorDelegate = new VertexEditorDelegate(this);

    areaEditor = new AreaSourceEditor;
    areaCircEditor = new AreaCircSourceEditor;
    areaPolyEditor = qobject_cast<VertexEditor *>(
        areaPolyEditorDelegate->createEditor(nullptr, QStyleOptionViewItem(), QModelIndex()));

    mapper = new QDataWidgetMapper(this);
    mapper->setOrientation(Qt::Horizontal);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);

    stack = new QStackedLayout;
    stack->setContentsMargins(0, 0, 0, 0);
    stack->addWidget(statusLabel);
    stack->addWidget(areaEditor);
    stack->addWidget(areaCircEditor);
    stack->addWidget(areaPolyEditor);

    btnUpdate = new QPushButton(tr("Update"));
    btnUpdate->setAutoDefault(true);
    btnUpdate->setDisabled(true);

    QHBoxLayout *buttonBox = new QHBoxLayout;
    buttonBox->setContentsMargins(0, 0, 0, 0);
    buttonBox->addStretch(1);
    buttonBox->addWidget(btnUpdate);

    QVBoxLayout *controlsLayout = new QVBoxLayout;
    controlsLayout->setContentsMargins(24, 16, 24, 16);
    controlsLayout->addLayout(stack);
    controlsLayout->addSpacing(5);
    controlsLayout->addLayout(buttonBox);

    ctkCollapsibleGroupBox *gbGeometry = new ctkCollapsibleGroupBox(tr("Geometry"));
    gbGeometry->setLayout(controlsLayout);
    gbGeometry->setFlat(true);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(gbGeometry);
    mainLayout->addStretch(1);

    stack->setCurrentIndex(0);

    connect(btnUpdate, &QPushButton::pressed, mapper, &QDataWidgetMapper::submit);

    setLayout(mainLayout);
}

void SourceEditor::setModel(QAbstractItemModel *model)
{
    if (model == nullptr)
        return;

    sourceModel = model;
    mapper->setModel(model);
}

void SourceEditor::setCurrentModelIndex(const QModelIndex& index)
{
    if (!sourceModel) {
        setStatusText("Invalid source data.");
        return;
    }

    if (!index.isValid() || index.model() == nullptr) {
        setStatusText("Invalid source selected.");
        return;
    }

    QVariant sourceTypeVar = index.model()->headerData(index.row(), Qt::Vertical, Qt::UserRole);
    if (!sourceTypeVar.canConvert<SourceType>()) {
        setStatusText("Invalid source type.");
        return;
    }

    int prevStackIndex = stack->currentIndex();

    SourceType sourceType = qvariant_cast<SourceType>(sourceTypeVar);
    switch (sourceType) {
    case SourceType::AREA:
        if (prevStackIndex != 1) {
            mapper->clearMapping();
            mapper->addMapping(areaEditor->sbXCoord, SourceModel::Column::X);
            mapper->addMapping(areaEditor->sbYCoord, SourceModel::Column::Y);
            mapper->addMapping(areaEditor->sbXInit, SourceModel::Column::XInit);
            mapper->addMapping(areaEditor->sbYInit, SourceModel::Column::YInit);
            mapper->addMapping(areaEditor->sbAngle, SourceModel::Column::Angle);
            mapper->toFirst();
        }
        stack->setCurrentIndex(1);
        mapper->setCurrentIndex(index.row());
        btnUpdate->setDisabled(false);
        break;
    case SourceType::AREACIRC:
        if (prevStackIndex != 2) {
            mapper->clearMapping();
            mapper->addMapping(areaCircEditor->sbXCoord, SourceModel::Column::X);
            mapper->addMapping(areaCircEditor->sbYCoord, SourceModel::Column::Y);
            mapper->addMapping(areaCircEditor->sbRadius, SourceModel::Column::Radius);
            mapper->addMapping(areaCircEditor->sbVertexCount, SourceModel::Column::VertexCount);
            mapper->toFirst();
        }
        stack->setCurrentIndex(2);
        mapper->setCurrentIndex(index.row());
        btnUpdate->setDisabled(false);
        break;
    case SourceType::AREAPOLY:
        if (prevStackIndex != 3) {
            mapper->clearMapping();
            mapper->addMapping(areaPolyEditor, 0);
            mapper->setItemDelegate(areaPolyEditorDelegate);
            mapper->toFirst();
        }
        stack->setCurrentIndex(3);
        mapper->setCurrentIndex(index.row());
        btnUpdate->setDisabled(false);
        break;
    default:
        setStatusText("Unsupported source type.");
        break;
    }
}

void SourceEditor::setStatusText(const QString& text)
{
    statusLabel->setText(text);
    btnUpdate->setDisabled(true);
    mapper->clearMapping();
    stack->setCurrentIndex(0);
}

/****************************************************************************
** AREA
****************************************************************************/

AreaSourceEditor::AreaSourceEditor(QWidget *parent) : QWidget(parent)
{
    sbXCoord = new QDoubleSpinBox();
    sbXCoord->setRange(-10000000, 10000000);
    sbXCoord->setDecimals(2);
    sbXCoord->setButtonSymbols(QAbstractSpinBox::NoButtons);

    sbYCoord = new QDoubleSpinBox();
    sbYCoord->setRange(-10000000, 10000000);
    sbYCoord->setDecimals(2);
    sbYCoord->setButtonSymbols(QAbstractSpinBox::NoButtons);

    sbXInit = new QDoubleSpinBox();
    sbXInit->setRange(0, 10000);
    sbXInit->setDecimals(2);
    sbXInit->setButtonSymbols(QAbstractSpinBox::NoButtons);

    sbYInit = new QDoubleSpinBox();
    sbYInit->setRange(0, 10000);
    sbYInit->setDecimals(2);
    sbYInit->setButtonSymbols(QAbstractSpinBox::NoButtons);

    sbAngle = new QDoubleSpinBox;
    sbAngle->setMinimum(0);
    sbAngle->setMaximum(359.9);
    sbAngle->setSingleStep(1);
    sbAngle->setDecimals(1);
    sbAngle->setWrapping(true);

    // Layout
    QGridLayout *layout1 = new QGridLayout;
    layout1->setContentsMargins(0,0,0,0);
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
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addLayout(layout1);
    mainLayout->addStretch(1);

    setLayout(mainLayout);
}

/****************************************************************************
** AREACIRC
****************************************************************************/

AreaCircSourceEditor::AreaCircSourceEditor(QWidget *parent) : QWidget(parent)
{
    sbXCoord = new QDoubleSpinBox();
    sbXCoord->setRange(-10000000, 10000000);
    sbXCoord->setDecimals(2);
    sbXCoord->setButtonSymbols(QAbstractSpinBox::NoButtons);

    sbYCoord = new QDoubleSpinBox();
    sbYCoord->setRange(-10000000, 10000000);
    sbYCoord->setDecimals(2);
    sbYCoord->setButtonSymbols(QAbstractSpinBox::NoButtons);

    sbRadius = new QDoubleSpinBox();
    sbRadius->setRange(0, 10000);
    sbRadius->setDecimals(2);
    sbRadius->setButtonSymbols(QAbstractSpinBox::NoButtons);

    sbVertexCount = new QSpinBox;
    sbVertexCount->setRange(3, 100);

    // Layout
    QGridLayout *layout1 = new QGridLayout;
    layout1->setContentsMargins(0,0,0,0);
    layout1->setColumnMinimumWidth(0, 120);
    layout1->addWidget(new QLabel(tr("Coordinates:")), 0, 0);
    layout1->addWidget(sbXCoord, 0, 1);
    layout1->addWidget(sbYCoord, 0, 2);
    layout1->addWidget(new QLabel(tr("Radius:")), 1, 0);
    layout1->addWidget(sbRadius, 1, 1, 1, 2);
    layout1->addWidget(new QLabel(tr("Vertex count:")), 2, 0);
    layout1->addWidget(sbVertexCount, 2, 1, 1, 2);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addLayout(layout1);
    mainLayout->addStretch(1);

    setLayout(mainLayout);
}

