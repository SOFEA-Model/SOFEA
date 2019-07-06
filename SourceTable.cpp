#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QIcon>
#include <QMenu>
#include <QDebug>

#include "SourceTable.h"
#include "SourceEditor.h"
#include "FluxProfilePlot.h"

class SourceEditorDialog : public QDialog
{
public:
    SourceEditorDialog(Source *s, QWidget *parent = nullptr) : QDialog(parent)
    {
        setAttribute(Qt::WA_DeleteOnClose, true);
        setWindowFlag(Qt::Tool);
        setWindowTitle(QString::fromStdString(s->srcid));

        SourceEditor *editor = new SourceEditor;
        editor->setSource(s);
        editor->setVisible(true);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
        connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

        QVBoxLayout *mainLayout = new QVBoxLayout;
        mainLayout->addWidget(editor);
        mainLayout->addWidget(buttonBox);
        setLayout(mainLayout);
    }
};

SourceTable::SourceTable(Scenario *s, SourceGroup *sg, QWidget *parent)
    : QWidget(parent), sPtr(s), sgPtr(sg)
{
    model = new SourceModel(s, sgPtr, this);

    table = new StandardTableView;
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::ContiguousSelection);
    table->verticalHeader()->setVisible(true);
    table->horizontalHeader()->setVisible(true);
    table->horizontalHeader()->setStretchLastSection(false);
    table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    table->setFrameStyle(QFrame::NoFrame);
    table->setContextMenuPolicy(Qt::CustomContextMenu);
    table->setModel(model);

    // NOTE: Format should be the same as ReceptorDialog and SourceGroupPages
    table->setDoubleSpinBoxForColumn(1, -10000000, 10000000, 2, 1);
    table->setDoubleSpinBoxForColumn(2, -10000000, 10000000, 2, 1);
    table->setDateTimeEditForColumn(4);
    table->setDoubleSpinBoxForColumn(5, 0, 10000000, 2, 1);
    table->setDoubleSpinBoxForColumn(6, 0, 100, 2, 1);

    fpEditorModel = new FluxProfileModel(this);
    fpEditorModel->load(s->fluxProfiles);
    ComboBoxDelegate *fpEditorDelegate = new ComboBoxDelegate(fpEditorModel, 0);
    table->setItemDelegateForColumn(8, fpEditorDelegate);

    massLabel = new QLabel;
    massLabel->setText(QString(" Total applied mass with AF: %1 kg").arg(getTotalMass()));

    // Context Menu Actions
    const QIcon icoArea = QIcon(":/images/Rectangle_16x.png");
    const QIcon icoAreaCirc = QIcon(":/images/Circle_16x.png");
    const QIcon icoAreaPoly = QIcon(":/images/Polygon_16x.png");
    const QIcon icoImport = QIcon(":/images/Import_grey_16x.png");
    const QIcon icoEdit = QIcon(":/images/Edit_grey_16x.png");
    const QIcon icoFlux = QIcon(":/images/KagiChart_16x.png");

    actAddArea = new QAction(icoArea, "Area", this);
    actAddAreaCirc = new QAction(icoAreaCirc, "Circular", this);
    actAddAreaPoly = new QAction(icoAreaPoly, "Polygon", this);
    actImport = new QAction(icoImport, "Import...", this);
    actEdit = new QAction(icoEdit, tr("Edit Geometry..."), this);
    actFlux = new QAction(icoFlux, tr("Plot Flux Profile..."), this);
    actRemove = new QAction(tr("Remove"), this);
    actResampleAppStart = new QAction(tr("Application Start"), this);
    actResampleAppRate = new QAction(tr("Application Rate"), this);
    actResampleIncorpDepth = new QAction(tr("Incorporation Depth"), this);
    actResampleFluxProfile = new QAction(tr("Flux Profile"), this);

    // Connections
    connect(table, &QTableView::customContextMenuRequested,
            this, &SourceTable::contextMenuRequested);

    QHeaderView *header = table->horizontalHeader();
    header->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(header, &QHeaderView::customContextMenuRequested,
            this, &SourceTable::headerContextMenuRequested);

    connect(model, &SourceModel::dataChanged, this, &SourceTable::handleDataChanged);
    connect(model, &SourceModel::rowsInserted, this, &SourceTable::handleRowsInserted);
    connect(model, &SourceModel::rowsRemoved, this, &SourceTable::handleRowsRemoved);

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 5);
    mainLayout->addWidget(table, 1);
    mainLayout->addWidget(massLabel);
    setLayout(mainLayout);
}

double SourceTable::getTotalMass() const
{
    // Calculate the total mass with AF.
    double total = 0;
    for (const Source &s : sgPtr->sources)
        total += sgPtr->appFactor * s.appRate * s.area();

    return total;
}

void SourceTable::plotFluxProfile(const Source *s)
{
    std::shared_ptr<FluxProfile> fp = s->fluxProfile.lock();
    if (!fp)
        return;

    FluxProfilePlot *plotWidget = new FluxProfilePlot(*fp);
    plotWidget->setAppStart(s->appStart);
    plotWidget->setAppRate(s->appRate);
    plotWidget->setIncorpDepth(s->incorpDepth);
    plotWidget->setControlsEnabled(false);
    plotWidget->setupConnections();
    plotWidget->updatePlot();

    QDialog *plotDialog = new QDialog(this);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::rejected, plotDialog, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(plotWidget);
    mainLayout->addWidget(buttonBox);

    plotDialog->setWindowTitle(QString::fromStdString(s->srcid));
    plotDialog->setWindowFlag(Qt::Tool);
    plotDialog->setAttribute(Qt::WA_DeleteOnClose);
    plotDialog->setLayout(mainLayout);
    plotDialog->exec();
}

void SourceTable::refresh()
{
    fpEditorModel->load(sPtr->fluxProfiles);
    massLabel->setText(QString(" Total applied mass with AF: %1 kg").arg(getTotalMass()));
}

void SourceTable::headerContextMenuRequested(const QPoint &pos)
{
    QMenu *contextMenu = new QMenu(this);

    for (int col = 0; col < model->columnCount(); ++col) {
        QString headerText = model->headerData(col, Qt::Horizontal, Qt::DisplayRole).toString();
        QAction *visibilityAct = new QAction(headerText);
        visibilityAct->setCheckable(true);
        visibilityAct->setChecked(isColumnVisible(col));
        contextMenu->addAction(visibilityAct);
    }

    QHeaderView *header = table->horizontalHeader();
    QPoint globalPos = header->viewport()->mapToGlobal(pos);
    QAction *selectedItem = contextMenu->exec(globalPos);

    if (selectedItem) {
        int selectedCol = contextMenu->actions().indexOf(selectedItem);
        setColumnVisible(selectedCol, selectedItem->isChecked());
    }

    contextMenu->deleteLater();
}

void SourceTable::contextMenuRequested(const QPoint &pos)
{
    QMenu *contextMenu = new QMenu(this);
    const QIcon icoAdd = QIcon(":/images/Add_grey_16x.png");
    const QIcon icoResample = QIcon(":/MeasureCalculate_16x.png");
    QMenu *addSourceMenu = contextMenu->addMenu(icoAdd, "Add Source");
    addSourceMenu->addAction(actAddArea);
    addSourceMenu->addAction(actAddAreaCirc);
    addSourceMenu->addAction(actAddAreaPoly);
    QMenu *resampleMenu = contextMenu->addMenu(icoResample, "Resample");
    resampleMenu->addAction(actResampleAppStart);
    resampleMenu->addAction(actResampleAppRate);
    resampleMenu->addAction(actResampleIncorpDepth);
    resampleMenu->addAction(actResampleFluxProfile);
    contextMenu->addAction(actImport);
    contextMenu->addAction(actEdit);
    contextMenu->addAction(actFlux);
    contextMenu->addSeparator();
    contextMenu->addAction(actRemove);

    QModelIndexList selected = table->selectionModel()->selectedIndexes();
    QModelIndexList selectedRows = table->selectionModel()->selectedRows();
    QModelIndex index = table->indexAt(pos);

    // Enable or disable actions depending on selection state.
    bool validSelection = index.isValid() && selected.contains(index);
    bool singleSelection = selectedRows.size() == 1;
    bool validFlux = false;
    Source *currentSource = model->sourceFromIndex(index);
    if (currentSource) {
        auto fp = currentSource->fluxProfile.lock();
        validFlux = fp && !fp->refFlux.empty();
    }

    resampleMenu->setEnabled(validSelection);
    actRemove->setEnabled(validSelection);
    actEdit->setEnabled(validSelection && singleSelection);
    actFlux->setEnabled(validSelection && singleSelection && validFlux);

    // Execute the context menu.
    QPoint globalPos = table->viewport()->mapToGlobal(pos);
    QAction *selectedItem = contextMenu->exec(globalPos);

    if (selectedItem)
    {
        if (selectedItem == actAddArea) {
            model->addAreaSource();
        }
        else if (selectedItem == actAddAreaCirc) {
            model->addAreaCircSource();
        }
        else if (selectedItem == actAddAreaPoly) {
            model->addAreaPolySource();
        }
        else if (selectedItem == actResampleAppStart) {
            for (const QModelIndex& index : selectedRows) {
                Source *s = model->sourceFromIndex(index);
                sgPtr->initSourceAppStart(s);
            }
        }
        else if (selectedItem == actResampleAppRate) {
            for (const QModelIndex& index : selectedRows) {
                Source *s = model->sourceFromIndex(index);
                sgPtr->initSourceAppRate(s);
            }
        }
        else if (selectedItem == actResampleIncorpDepth) {
            for (const QModelIndex& index : selectedRows) {
                Source *s = model->sourceFromIndex(index);
                sgPtr->initSourceIncorpDepth(s);
            }
        }
        else if (selectedItem == actResampleFluxProfile) {
            for (const QModelIndex& index : selectedRows) {
                Source *s = model->sourceFromIndex(index);
                sgPtr->initSourceFluxProfile(s);
            }
        }
        else if (selectedItem == actImport) {
            model->import();
        }
        else if (selectedItem == actEdit) {
            SourceEditorDialog *dialog = new SourceEditorDialog(currentSource, this);
            dialog->exec();
            currentSource->setGeometry();
            emit dataChanged();
        }
        else if (selectedItem == actFlux) {
            plotFluxProfile(currentSource);
        }
        else if (selectedItem == actRemove) {
            table->removeSelectedRows();
        }
    }

    contextMenu->deleteLater();
}

void SourceTable::handleDataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)
{
    massLabel->setText(QString(" Total applied mass with AF: %1 kg").arg(getTotalMass()));
    emit dataChanged();
}

void SourceTable::handleRowsInserted(const QModelIndex &, int, int)
{
    massLabel->setText(QString(" Total applied mass with AF: %1 kg").arg(getTotalMass()));
    emit dataChanged();
}

void SourceTable::handleRowsRemoved(const QModelIndex &, int, int)
{
    massLabel->setText(QString(" Total applied mass with AF: %1 kg").arg(getTotalMass()));
    emit dataChanged();
}

void SourceTable::setColumnVisible(int column, bool visible)
{
    table->setColumnHidden(column, !visible);
    model->setColumnHidden(column, !visible);
}

bool SourceTable::isColumnVisible(int column) const
{
    return !model->isColumnHidden(column);
}
