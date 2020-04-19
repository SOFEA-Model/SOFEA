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
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>

#include <QDebug>

#include "SamplingDistributionEditor.h"
#include "delegates/DoubleSpinBoxDelegate.h"

SamplingDistributionEditor::SamplingDistributionEditor(QAbstractItemModel *sourceModel, QWidget *parent)
    : QWidget(parent)
{
    if (sourceModel == nullptr)
        return;

    proxyModel = new SamplingProxyModel;
    proxyModel->setSourceModel(sourceModel);

    view = new StandardTableView;
    view->setModel(proxyModel);
    view->setSelectionBehavior(QAbstractItemView::SelectItems);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setEditTriggers(QAbstractItemView::AllEditTriggers);

    int lastColumn = proxyModel->proxyColumnForExtraColumn(0);
    view->setItemDelegateForColumn(lastColumn, new DoubleSpinBoxDelegate(0, 1, 4, 0.1));

    QHeaderView *header = view->horizontalHeader();
    header->setStretchLastSection(false);
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setSectionResizeMode(lastColumn, QHeaderView::Fixed);
    header->resizeSection(lastColumn, 100);

    btnNormalize = new QPushButton("Normalize");

    // Connections
    connect(btnNormalize, &QPushButton::clicked, this, &SamplingDistributionEditor::normalize);

    // Layout
    QHBoxLayout *controlsLayout = new QHBoxLayout;
    controlsLayout->setMargin(0);
    controlsLayout->addStretch(1);
    controlsLayout->addWidget(btnNormalize);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->addWidget(view);
    mainLayout->addSpacing(5);
    mainLayout->addLayout(controlsLayout);

    setLayout(mainLayout);
}

void SamplingDistributionEditor::setColumnHidden(int column)
{
    view->setColumnHidden(column, true);
}

bool SamplingDistributionEditor::setProbability(int row, double value)
{
    int column = proxyModel->proxyColumnForExtraColumn(0);
    QModelIndex index = proxyModel->index(row, column);
    return proxyModel->setData(index, value);
}

double SamplingDistributionEditor::probability(int row) const
{
    int column = proxyModel->proxyColumnForExtraColumn(0);
    QModelIndex index = proxyModel->index(row, column);
    return proxyModel->data(index).toDouble();
}

void SamplingDistributionEditor::normalize()
{
    proxyModel->normalize();
}
