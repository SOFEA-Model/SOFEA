#include "BufferZoneEditor.h"
#include "delegates/DoubleSpinBoxDelegate.h"
#include "delegates/SpinBoxDelegate.h"
#include "models/BufferZoneModel.h"
#include "widgets/GridLayout.h"
#include "widgets/StandardTableView.h"
#include "widgets/StatusLabel.h"

#include <stdexcept>

#include <csv/csv.h>

#include <QBoxLayout>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>

BufferZoneEditor::BufferZoneEditor(BufferZoneModel *m, QWidget *parent)
    : QWidget(parent), model(m)
{
    chkEnable = new QCheckBox("Enable buffer zone processing");

    sbAreaThreshold = new QDoubleSpinBox;
    sbAreaThreshold->setRange(0, 10000);

    sbAppRateThreshold = new QDoubleSpinBox;
    sbAppRateThreshold->setRange(0, 10000000);
    sbAppRateThreshold->setDecimals(2);

    btnAdd = new QPushButton(tr("Add"));
    btnRemove = new QPushButton(tr("Remove"));
    btnRemove->setEnabled(false);
    btnImport = new QPushButton(tr("Import..."));

    table = new StandardTableView;
    table->setModel(model);
    table->setItemDelegateForColumn(BufferZoneModel::Column::Distance, new DoubleSpinBoxDelegate(0, 99000, 2, 1));
    table->setItemDelegateForColumn(BufferZoneModel::Column::Duration, new SpinBoxDelegate(0, 1000, 1));
    table->setSelectionBehavior(QAbstractItemView::SelectItems);
    table->setSelectionMode(QAbstractItemView::ContiguousSelection);
    table->setEditTriggers(QAbstractItemView::AllEditTriggers);

    int startingWidth = table->font().pointSize();
    int frameWidth = table->frameWidth();
    table->setColumnWidth(0, startingWidth * 10);
    table->setColumnWidth(1, startingWidth * 20);
    table->setColumnWidth(2, startingWidth * 15);
    table->setColumnWidth(3, startingWidth * 15);
    table->setMinimumWidth(startingWidth * 60 + frameWidth * 2);

    lblThresholdInfo = new StatusLabel;
    lblThresholdInfo->setStatusType(StatusLabel::InfoTip);
    lblThresholdInfo->setText(
        "The buffer zone assigned to a source is the last row for which area "
        "and application rate are less than the threshold values.");

    // Connections
    connect(btnAdd, &QPushButton::clicked, this, &BufferZoneEditor::onAddClicked);
    connect(btnRemove, &QPushButton::clicked, this, &BufferZoneEditor::onRemoveClicked);
    connect(btnImport, &QPushButton::clicked, this, &BufferZoneEditor::onImportClicked);
    connect(table->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &BufferZoneEditor::onSelectionChanged);

    // Layout
    GridLayout *inputLayout = new GridLayout;
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->addWidget(new QLabel(tr("Area threshold (ha):")), 0, 0);
    inputLayout->addWidget(sbAreaThreshold, 0, 1);
    inputLayout->addWidget(new QLabel(tr("App. rate threshold (kg/ha):")), 1, 0);
    inputLayout->addWidget(sbAppRateThreshold, 1, 1);

    QVBoxLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(btnAdd);
    buttonLayout->addWidget(btnRemove);
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(btnImport);
    buttonLayout->addStretch(1);

    QHBoxLayout *tableLayout = new QHBoxLayout;
    tableLayout->setContentsMargins(0, 0, 0, 0);
    tableLayout->addWidget(table);
    tableLayout->addLayout(buttonLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(chkEnable);
    mainLayout->addSpacing(5);
    mainLayout->addLayout(inputLayout);
    mainLayout->addSpacing(5);
    mainLayout->addLayout(tableLayout);
    mainLayout->addStretch(1);
    mainLayout->addWidget(lblThresholdInfo);
    mainLayout->addSpacing(5);

    setLayout(mainLayout);
}

bool BufferZoneEditor::isEnableChecked()
{
    return chkEnable->isChecked();
}

void BufferZoneEditor::setEnableChecked(bool checked)
{
    chkEnable->setChecked(checked);
}

void BufferZoneEditor::onAddClicked()
{
    BufferZone z;
    z.areaThreshold = sbAreaThreshold->value();
    z.appRateThreshold = sbAppRateThreshold->value();
    model->insert(z);
}

void BufferZoneEditor::onRemoveClicked()
{
    table->removeSelectedRows();
}

void BufferZoneEditor::onImportClicked()
{
    QSettings settings;
    QString filename = QFileDialog::getOpenFileName(this,
                       tr("Import Buffer Zone Table"),
                       QDir::currentPath(),
                       tr("Buffer Zone Table (*.csv *.txt)"));

    if (filename.isEmpty())
        return;

    std::set<BufferZone> imported;

    try {
        io::CSVReader<4> in(filename.toStdString());
        in.read_header(io::ignore_extra_column, "area", "apprate", "distance", "duration");
        while (true) {
            BufferZone z;
            if (!in.read_row(z.areaThreshold, z.appRateThreshold, z.distance, z.duration))
                break;
            if (imported.find(z) != imported.end())
                throw std::runtime_error("File contains duplicate buffer zones.");
            imported.insert(z);
        }
        model->load(imported);
    }
    catch (const std::exception &e) {
        QMessageBox::critical(this, "Import Failed", QString::fromLocal8Bit(e.what()));
    }
}

void BufferZoneEditor::onSelectionChanged(const QItemSelection&, const QItemSelection&)
{
    bool hasSelection = table->selectionModel()->hasSelection();
    btnRemove->setEnabled(hasSelection);
}

