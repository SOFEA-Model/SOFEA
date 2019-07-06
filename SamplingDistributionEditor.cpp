#include <QBoxLayout>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>

#include <QDebug>

#include "SamplingDistributionEditor.h"

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
    view->setDoubleSpinBoxForColumn(lastColumn, 0, 1, 4, 0.1);

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

double SamplingDistributionEditor::getProbability(int row) const
{
    int column = proxyModel->proxyColumnForExtraColumn(0);
    QModelIndex index = proxyModel->index(row, column);
    return proxyModel->data(index).toDouble();
}

void SamplingDistributionEditor::normalize()
{
    proxyModel->normalize();
}
