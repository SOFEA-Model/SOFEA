#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QIcon>
#include <QMenu>
#include <QDebug>

#include "SourceTable.h"
#include "SourceEditor.h"
#include "StandardPlot.h"

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

SourceTable::SourceTable(SourceGroup *sg, QWidget *parent)
    : QWidget(parent), sgPtr(sg)
{
    model = new SourceModel(sgPtr, this);

    table = new StandardTableView;
    table->setSelectionMode(QAbstractItemView::ContiguousSelection);
    table->verticalHeader()->setVisible(true);
    table->horizontalHeader()->setVisible(true);
    table->horizontalHeader()->setStretchLastSection(false);
    table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    table->setFrameStyle(QFrame::NoFrame);
    table->setContextMenuPolicy(Qt::CustomContextMenu);
    table->setModel(model);

    // NOTE: format should be the same as SourceGroupPages
    table->setDoubleSpinBoxForColumn(1, -10000000, 10000000, 2, true);
    table->setDoubleSpinBoxForColumn(2, -10000000, 10000000, 2, true);
    table->setDateTimeEditForColumn(4);
    table->setDoubleSpinBoxForColumn(5, 0, 10000000, 2, true);
    table->setDoubleSpinBoxForColumn(6, 0, 100, 2, true);

    massLabel = new QLabel;
    massLabel->setText(QString(" Total applied mass with AF: %1 kg").arg(getTotalMass()));

    // Context Menu Actions
    const QIcon icoArea = QIcon(":/images/Rectangle_16x.png");
    const QIcon icoAreaCirc = QIcon(":/images/Circle_16x.png");
    const QIcon icoAreaPoly = QIcon(":/images/Polygon_16x.png");
    const QIcon icoImport = QIcon(":/images/Import_grey_16x.png");
    const QIcon icoEdit = QIcon(":/images/Edit_grey_16x.png");
    const QIcon icoFlux = QIcon(":/images/KagiChart_16x.png");
    const QIcon icoRefresh = QIcon(":/images/Refresh_16x.png");
    actAddArea = new QAction(icoArea, "Area", this);
    actAddAreaCirc = new QAction(icoAreaCirc, "Circular", this);
    actAddAreaPoly = new QAction(icoAreaPoly, "Polygon", this);
    actImport = new QAction(icoImport, "Import...", this);
    actEdit = new QAction(icoEdit, tr("Edit Geometry"), this);
    actFlux = new QAction(icoFlux, tr("Flux Profile"), this);
    actRemove = new QAction(tr("Remove"), this);
    actRefresh = new QAction(icoRefresh, tr("Refresh"), this);

    // Connections
    connect(table, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showContextMenu(const QPoint &)));
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

void SourceTable::showFluxProfile(const Source *s)
{
    if (sgPtr->refFlux.size() == 0)
        return;

    Flux flux = sgPtr->fluxProfile(s);

    QVector<QPointF> series;
    series.reserve(flux.size() + 1);

    for (const auto& xy : flux) {
        double x = QwtDate::toDouble(xy.first);
        double y = xy.second;
        QPointF p(x, y);
        series.push_back(p);
    }

    // Add the endpoint for the step function (+1 hour).
    QDateTime dtn = flux.back().first;
    dtn = dtn.addSecs(60 * 60);
    double xn = QwtDate::toDouble(dtn);
    double yn = flux.back().second;
    QPointF pn(xn, yn);
    series.push_back(pn);

    // Draw the step function.
    StandardPlot *plot = new StandardPlot;
    plot->setCurveTracker(true);
    QwtPlotCurve *curve = new QwtPlotCurve;
    curve->setStyle(QwtPlotCurve::Steps);
    curve->setCurveAttribute(QwtPlotCurve::Inverted);
    curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    curve->setSamples(series);
    curve->attach(plot);

    QwtDateScaleDraw *scaleDraw = new QwtDateScaleDraw;
    scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    QwtDateScaleEngine *scaleEngine = new QwtDateScaleEngine;
    plot->setAxisScaleDraw(QwtPlot::xBottom, scaleDraw);
    plot->setAxisScaleEngine(QwtPlot::xBottom, scaleEngine);

    BackgroundFrame *plotFrame = new BackgroundFrame;
    QVBoxLayout *plotLayout = new QVBoxLayout;
    plotLayout->addWidget(plot);
    plotFrame->setLayout(plotLayout);

    QDialog *plotWindow = new QDialog(this);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::rejected, plotWindow, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(plotFrame);
    mainLayout->addWidget(buttonBox);

    plotWindow->setWindowTitle(QString::fromStdString(s->srcid));
    plotWindow->setWindowFlag(Qt::Tool);
    plotWindow->setAttribute(Qt::WA_DeleteOnClose);
    plotWindow->setLayout(mainLayout);
    plotWindow->exec();
}

void SourceTable::refresh()
{
    massLabel->setText(QString(" Total applied mass with AF: %1 kg").arg(getTotalMass()));
}

void SourceTable::showContextMenu(const QPoint &pos)
{
    QMenu *contextMenu = new QMenu(this);
    const QIcon icoAdd = QIcon(":/images/Add_grey_16x.png");
    QMenu *addSourceMenu = contextMenu->addMenu(icoAdd, "Add Source");
    addSourceMenu->addAction(actAddArea);
    addSourceMenu->addAction(actAddAreaCirc);
    addSourceMenu->addAction(actAddAreaPoly);
    contextMenu->addAction(actImport);
    contextMenu->addAction(actEdit);
    contextMenu->addAction(actFlux);
    contextMenu->addSeparator();
    contextMenu->addAction(actRemove);
    contextMenu->addSeparator();
    contextMenu->addAction(actRefresh);

    QModelIndexList selected = table->selectionModel()->selectedIndexes();
    QModelIndexList selectedRows = table->selectionModel()->selectedRows();
    QModelIndex index = table->indexAt(pos);
    if (index.isValid() && selected.contains(index)) {
        actRemove->setDisabled(false);
        if (selectedRows.size() == 1) {
            actEdit->setDisabled(false);
            actFlux->setDisabled(false);
        }
        else {
            actEdit->setDisabled(true);
            actFlux->setDisabled(true);
        }
    }
    else {
        actRemove->setDisabled(true);
        actEdit->setDisabled(true);
        actFlux->setDisabled(true);
    }

    if (sgPtr->refFlux.size() == 0)
        actFlux->setDisabled(true);

    // Execute the context menu.
    QPoint globalPos = table->viewport()->mapToGlobal(pos);
    QAction *selectedItem = contextMenu->exec(globalPos);
    if (selectedItem && selectedItem == actAddArea) {
        model->addAreaSource();
    }
    else if (selectedItem && selectedItem == actAddAreaCirc) {
        model->addAreaCircSource();
    }
    else if (selectedItem && selectedItem == actAddAreaPoly) {
        model->addAreaPolySource();
    }
    else if (selectedItem && selectedItem == actImport) {
        model->import();
    }
    else if (selectedItem && selectedItem == actEdit) {
        Source *s = model->getSource(selectedRows.first());
        SourceEditorDialog *dialog = new SourceEditorDialog(s, this);
        dialog->exec();
        s->setGeometry();
        emit dataChanged();
    }
    else if (selectedItem && selectedItem == actFlux) {
        Source *s = model->getSource(selectedRows.first());
        showFluxProfile(s);
    }
    else if (selectedItem && selectedItem == actRemove) {
        table->removeSelectedRows();
    }
    else if (selectedItem && selectedItem == actRefresh) {
        refresh();
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
