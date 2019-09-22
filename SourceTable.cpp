#include <QColorDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QIcon>
#include <QMenu>
#include <QDebug>

#include <algorithm>

#include "SourceTable.h"
#include "SourceEditor.h"
#include "FluxProfilePlot.h"
#include "Projection.h"
#include "delegate/ComboBoxDelegate.h"

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

        QVBoxLayout *mainLayout = new QVBoxLayout;
        mainLayout->addWidget(editor);
        setLayout(mainLayout);
    }
};

class SymbolHeaderView : public QHeaderView
{
public:
    SymbolHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr)
        : QHeaderView(orientation, parent)
    {
        setSectionsClickable(true);
        setSectionResizeMode(QHeaderView::Fixed);
        setDefaultSectionSize(24);
        setFixedWidth(36);
    }

protected:
    void paintSection(QPainter *painter, const QRect &rect, int section) const override
    {
        painter->save();
        QStyleOptionHeader opt;
        initStyleOption(&opt);
        opt.section = section;
        opt.rect = rect;
        style()->drawControl(QStyle::CE_HeaderSection, &opt, painter, this);
        painter->restore();

        static const QPointF polygon[6] = {
            QPointF(0.1875, 0.5000),
            QPointF(0.4375, 0.1875),
            QPointF(0.8125, 0.3750),
            QPointF(0.6250, 0.5625),
            QPointF(0.8125, 0.8125),
            QPointF(0.3125, 0.8125)
        };

        QAbstractItemModel *m = qobject_cast<QAbstractItemModel *>(model());
        if (m && orientation() == Qt::Vertical) {
            if (section >= m->rowCount() || section < 0)
                return;

            QVariant sourceTypeVar = m->headerData(section, Qt::Vertical, Qt::UserRole);
            QVariant foregroundVar = m->headerData(section, Qt::Vertical, Qt::ForegroundRole);
            QVariant backgroundVar = m->headerData(section, Qt::Vertical, Qt::BackgroundRole);

            SourceType sourceType = sourceTypeVar.value<SourceType>();
            QPen pen = foregroundVar.value<QPen>();
            QBrush brush = backgroundVar.value<QBrush>();
            pen.setCosmetic(true);

            int side = qMin(rect.width(), rect.height());
            int pm = style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, this);
            const int x = rect.center().x() - pm / 2;
            const int y = rect.center().y() - pm / 2;

            painter->save();
            painter->translate(x, y);
            painter->scale(pm, pm);
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->setPen(pen);
            painter->setBrush(brush);

            switch (sourceType) {
            case SourceType::AREA:
                painter->drawRect(QRectF{0.1875, 0.1875, 0.625, 0.625});
                break;
            case SourceType::AREACIRC:
                painter->drawEllipse(QPointF{0.5, 0.5}, 0.3125, 0.3125);
                break;
            case SourceType::AREAPOLY:
                painter->drawPolygon(polygon, sizeof polygon / sizeof polygon[0], Qt::WindingFill);
                break;
            }

            painter->restore();
        }
    }
};



SourceTable::SourceTable(Scenario *s, SourceGroup *sg, QWidget *parent)
    : QWidget(parent), sPtr(s), sgPtr(sg)
{
    model = new SourceModel(s, sgPtr, this);
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);

    table = new StandardTableView;
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    //table->setSelectionMode(QAbstractItemView::ExtendedSelection); // FIXME
    table->setSelectionMode(QAbstractItemView::ContiguousSelection);
    table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    table->setFrameStyle(QFrame::NoFrame);
    table->setContextMenuPolicy(Qt::CustomContextMenu);
    table->setModel(proxyModel);
    table->setAutoFilterEnabled(true);

    QHeaderView *horizontal = table->horizontalHeader();
    horizontal->setStretchLastSection(false);
    horizontal->setContextMenuPolicy(Qt::CustomContextMenu);
    horizontal->setVisible(true);

    QHeaderView *vertical = new SymbolHeaderView(Qt::Vertical, table);
    table->setVerticalHeader(vertical);
    vertical->setVisible(true);

    // NOTE: Format should be the same as ReceptorDialog and SourceGroupPages
    table->setDoubleLineEditForColumn(SourceModel::Column::X, -10000000, 10000000, 2, true);
    table->setDoubleLineEditForColumn(SourceModel::Column::Y, -10000000, 10000000, 2, true);
    table->setDoubleLineEditForColumn(SourceModel::Column::Z, 0, 9999.99, 2, true);
    table->setDoubleLineEditForColumn(SourceModel::Column::Longitude, -180.0, 180.0, 5, true);
    table->setDoubleLineEditForColumn(SourceModel::Column::Latitude, -90.0, 90.0, 5, true);
    table->setDateTimeEditForColumn(SourceModel::Column::Start);
    table->setDoubleSpinBoxForColumn(SourceModel::Column::AppRate, 0, 10000000, 2, 1);
    table->setDoubleSpinBoxForColumn(SourceModel::Column::IncDepth, 0, 100, 2, 1);

    fpEditorModel = new FluxProfileModel(this);
    ComboBoxDelegate *fpEditorDelegate = new ComboBoxDelegate(fpEditorModel, 0);
    table->setItemDelegateForColumn(SourceModel::Column::FluxProfile, fpEditorDelegate);

    massLabel = new QLabel;

    // Context Menu Actions
    static const QIcon icoArea = QIcon(":/images/Rectangle_16x.png");
    static const QIcon icoAreaCirc = QIcon(":/images/Circle_16x.png");
    static const QIcon icoAreaPoly = QIcon(":/images/Polygon_16x.png");
    static const QIcon icoImport = QIcon(":/images/Import_grey_16x.png");
    static const QIcon icoEdit = QIcon(":/images/Edit_grey_16x.png");
    static const QIcon icoColor = QIcon(":/images/ColorPalette_16x.png");
    static const QIcon icoFlux = QIcon(":/images/KagiChart_16x.png");

    actAddArea = new QAction(icoArea, "Area", this);
    actAddAreaCirc = new QAction(icoAreaCirc, "Circular", this);
    actAddAreaPoly = new QAction(icoAreaPoly, "Polygon", this);
    actImport = new QAction(icoImport, "Import...", this);
    actEdit = new QAction(icoEdit, tr("Edit Geometry..."), this);
    actColor = new QAction(icoColor, tr("Edit Colors..."), this);
    actFlux = new QAction(icoFlux, tr("Plot Flux Profile..."), this);
    actRemove = new QAction(tr("Remove"), this);
    actResampleAppStart = new QAction(tr("Application Start"), this);
    actResampleAppRate = new QAction(tr("Application Rate"), this);
    actResampleIncorpDepth = new QAction(tr("Incorporation Depth"), this);
    actResampleFluxProfile = new QAction(tr("Flux Profile"), this);

    // Connections
    connect(table, &QTableView::customContextMenuRequested,
            this, &SourceTable::contextMenuRequested);

    connect(horizontal, &QHeaderView::customContextMenuRequested,
            this, &SourceTable::headerContextMenuRequested);

    connect(vertical, &QHeaderView::sectionDoubleClicked, [=](int section) {
        QModelIndex proxyIndex = proxyModel->index(section, 0);
        QModelIndex index = proxyModel->mapToSource(proxyIndex);
        QModelIndexList selection;
        selection.push_back(index);
        openColorEditor(selection);
    });

    connect(model, &SourceModel::dataChanged, this, &SourceTable::handleDataChanged);
    connect(model, &SourceModel::rowsInserted, this, &SourceTable::handleRowsInserted);
    connect(model, &SourceModel::rowsRemoved, this, &SourceTable::handleRowsRemoved);

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 5);
    mainLayout->addWidget(table, 1);
    mainLayout->addWidget(massLabel);
    setLayout(mainLayout);

    refresh();
}

double SourceTable::getTotalMass() const
{
    // Calculate the total mass with AF.
    double total = 0;
    for (const Source &s : sgPtr->sources)
        total += sgPtr->appFactor * s.appRate * s.area();

    return total;
}

void SourceTable::refresh()
{
    model->setProjection(Projection::Generic(sPtr->conversionCode,
        sPtr->hDatumCode, sPtr->hUnitsCode, sPtr->vDatumCode, sPtr->vUnitsCode));

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
    static const QIcon icoAdd = QIcon(":/images/Add_grey_16x.png");
    static const QIcon icoResample = QIcon(":/Refresh_16x.png");

    QMenu *contextMenu = new QMenu(this);
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
    contextMenu->addAction(actColor);
    contextMenu->addAction(actFlux);
    contextMenu->addSeparator();
    contextMenu->addAction(actRemove);

    QModelIndexList selected = table->selectionModel()->selectedIndexes();
    QModelIndexList selectedRows = table->selectionModel()->selectedRows();
    QModelIndex proxyIndex = table->indexAt(pos);
    QModelIndex index = proxyModel->mapToSource(proxyIndex);

    // Enable or disable actions depending on selection state.
    bool validSelection = proxyIndex.isValid() && selected.contains(proxyIndex);
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
    actColor->setEnabled(validSelection);
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
            for (const QModelIndex& pi : selectedRows) {
                QModelIndex i = proxyModel->mapToSource(pi);
                Source *s = model->sourceFromIndex(i);
                sgPtr->initSourceAppStart(s);
            }
        }
        else if (selectedItem == actResampleAppRate) {
            for (const QModelIndex& pi : selectedRows) {
                QModelIndex i = proxyModel->mapToSource(pi);
                Source *s = model->sourceFromIndex(i);
                sgPtr->initSourceAppRate(s);
            }
        }
        else if (selectedItem == actResampleIncorpDepth) {
            for (const QModelIndex& pi : selectedRows) {
                QModelIndex i = proxyModel->mapToSource(pi);
                Source *s = model->sourceFromIndex(i);
                sgPtr->initSourceIncorpDepth(s);
            }
        }
        else if (selectedItem == actResampleFluxProfile) {
            for (const QModelIndex& pi : selectedRows) {
                QModelIndex i = proxyModel->mapToSource(pi);
                Source *s = model->sourceFromIndex(i);
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
        else if (selectedItem == actColor) {
            for (QModelIndex& i : selectedRows) {
                i = proxyModel->mapToSource(i);
            }
            openColorEditor(selectedRows);
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

QColor SourceTable::colorFromIndex(const QModelIndex &index) const
{
    QVariant background = model->headerData(index.row(), Qt::Vertical, Qt::BackgroundRole);
    if (background.canConvert<QBrush>()) {
        QBrush brush = qvariant_cast<QBrush>(background);
        return brush.color();
    }
    else {
        return QColor();
    }
}

void SourceTable::openColorEditor(const QModelIndexList& selection)
{
    if (model == nullptr)
        return;

    if (selection.size() == 0)
        return;

    // Set the initial color, using a default value if the selection has different colors.
    QColor color;
    if (selection.size() == 1) {
        color = colorFromIndex(selection.front());
    }
    else if (selection.size() > 1) {
        auto it = std::adjacent_find(selection.cbegin(), selection.cend(),
            [=](const QModelIndex& a, const QModelIndex& b) {
                return colorFromIndex(a) != colorFromIndex(b);
            });

        if (it == selection.end()) {
            color = colorFromIndex(selection.front());
        }
    }

    QColorDialog dialog;
    dialog.setOptions(QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
    dialog.setCurrentColor(color);

    int rc = dialog.exec();
    if (rc != QDialog::Accepted)
        return;

    for (const QModelIndex& index : selection) {
        QVariant background = model->headerData(index.row(), Qt::Vertical, Qt::BackgroundRole);
        QBrush brush;
        if (background.canConvert<QBrush>())
            brush = qvariant_cast<QBrush>(background);

        brush.setColor(dialog.currentColor());

        if (brush.style() == Qt::NoBrush)
            brush.setStyle(Qt::SolidPattern);

        model->setHeaderData(index.row(), Qt::Vertical, brush, Qt::BackgroundRole);
    }
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
