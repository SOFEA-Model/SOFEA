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

#include <QBoxLayout>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>

#include "VertexEditor.h"

VertexEditor::VertexEditor(QWidget *parent) : QWidget(parent)
{
    sbVertexCount = new QSpinBox;
    sbVertexCount->setRange(3, 5);
    sbVertexCount->setValue(5);

    sbVertex = new QSpinBox;
    sbVertex->setRange(1, 5);
    sbVertex->setWrapping(true);
    sbVertex->setValue(1);

    sbXCoord = new QDoubleSpinBox;
    sbXCoord->setRange(-10000000, 10000000);
    sbXCoord->setDecimals(2);
    sbXCoord->setButtonSymbols(QAbstractSpinBox::NoButtons);

    sbYCoord = new QDoubleSpinBox;
    sbYCoord->setRange(-10000000, 10000000);
    sbYCoord->setDecimals(2);
    sbYCoord->setButtonSymbols(QAbstractSpinBox::NoButtons);

    connect(sbVertexCount, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &VertexEditor::vertexCountChanged);

    connect(sbVertex, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &VertexEditor::vertexChanged);

    connect(sbXCoord, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &VertexEditor::coordinatesChanged);

    connect(sbYCoord, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &VertexEditor::coordinatesChanged);

    // Layout
    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->setColumnMinimumWidth(0, 120);
    gridLayout->addWidget(new QLabel(tr("Vertex count:")), 0, 0);
    gridLayout->addWidget(sbVertexCount, 0, 1, 1, 2);
    gridLayout->addWidget(new QLabel(tr("Selected vertex:")), 1, 0);
    gridLayout->addWidget(sbVertex, 1, 1, 1, 2);
    gridLayout->addWidget(new QLabel(tr("Coordinates:")), 2, 0);
    gridLayout->addWidget(sbXCoord, 2, 1);
    gridLayout->addWidget(sbYCoord, 2, 2);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(gridLayout);
    mainLayout->addStretch(1);

    setLayout(mainLayout);
}
