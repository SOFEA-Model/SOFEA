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

    sbProbability = new QDoubleSpinBox;
    sbProbability->setRange(0, 1);
    sbProbability->setValue(1);
    sbProbability->setDecimals(4);
    sbProbability->setSingleStep(0.01);

    btnSet = new QPushButton("Set");
    btnSet->setEnabled(false);

    proxyModel = new SamplingProxyModel;
    proxyModel->setSourceModel(sourceModel);

    view = new StandardTableView;
    view->setModel(proxyModel);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    int lastColumn = proxyModel->proxyColumnForExtraColumn(0);
    view->setDoubleLineEditForColumn(lastColumn, 0, 1, 4, true);

    QHeaderView *header = view->horizontalHeader();
    header->setStretchLastSection(false);
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setSectionResizeMode(lastColumn, QHeaderView::Fixed);
    header->resizeSection(lastColumn, 100);

    btnNormalize = new QPushButton("Normalize");

    // Connections
    connect(btnSet, &QPushButton::clicked, this, &SamplingDistributionEditor::onSetClicked);
    connect(btnNormalize, &QPushButton::clicked, this, &SamplingDistributionEditor::normalize);

    connect(view->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &SamplingDistributionEditor::onSelectionChanged);

    // Layout
    QHBoxLayout *inputLayout = new QHBoxLayout;
    inputLayout->addWidget(new QLabel("Probability: "), 0);
    inputLayout->addWidget(sbProbability, 1);
    inputLayout->addWidget(btnSet, 0);

    QHBoxLayout *controlsLayout = new QHBoxLayout;
    controlsLayout->setMargin(0);
    controlsLayout->addStretch(1);
    controlsLayout->addWidget(btnNormalize);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->addLayout(inputLayout);
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

void SamplingDistributionEditor::onSetClicked()
{
    int column = proxyModel->proxyColumnForExtraColumn(0);

    QModelIndexList selectedRows = view->selectionModel()->selectedRows();
    for (const QModelIndex& rowIndex : selectedRows) {
        QModelIndex index = rowIndex.siblingAtColumn(column);
        double value = sbProbability->value();
        proxyModel->setData(index, value);
    }
}

void SamplingDistributionEditor::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    bool hasSelection = view->selectionModel()->hasSelection();
    btnSet->setEnabled(hasSelection);
}
