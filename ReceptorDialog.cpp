#include <algorithm>
#include <tuple>

#include <QtWidgets>

#include "ReceptorDialog.h"
#include "Utilities.h"
#include "GeosOp.h"

template <typename T>
void addStandardItem(QStandardItemModel *model, int row, int col, T value, bool editable)
{
    QStandardItem *item = new QStandardItem;
    item->setEditable(editable);
    item->setData(value, Qt::DisplayRole);
    model->setItem(row, col, item);
}

//-----------------------------------------------------------------------------
// ReceptorRingTab
//-----------------------------------------------------------------------------

ReceptorRingTab::ReceptorRingTab(QStandardItemModel *model, QWidget *parent)
    : QWidget(parent), ringModel(model)
{
    cboSourceGroup = new QComboBox;

    sbRingBuffer = new QDoubleSpinBox;
    sbRingBuffer->setRange(0.1, 99000); // 99km
    sbRingBuffer->setValue(100);
    sbRingBuffer->setDecimals(2);

    sbRingSpacing = new QDoubleSpinBox;
    sbRingSpacing->setRange(0.1, 99000); // 99km
    sbRingSpacing->setValue(100);
    sbRingSpacing->setDecimals(2);

    ringTable = new StandardTableView;
    ringTable->setModel(ringModel);
    ringTable->setMinimumWidth(400);
    ringTable->setItemDelegateForColumn(0, new ColorPickerDelegate);
    ringTable->setDoubleLineEditForColumn(1, 0.1, 99000, 7, false);
    ringTable->setDoubleLineEditForColumn(2, 0.1, 99000, 7, false);
    ringTable->horizontalHeader()->setStretchLastSection(false);
    ringTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ringTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ringTable->setSelectionMode(QAbstractItemView::ContiguousSelection);

    ringEditor = new StandardTableEditor(Qt::Horizontal);

    // Connections
    connect(ringEditor->btnAdd,    &QPushButton::clicked, this, &ReceptorRingTab::onAddRingClicked);
    connect(ringEditor->btnRemove, &QPushButton::clicked, this, &ReceptorRingTab::onRemoveRingClicked);
    ringEditor->init(ringTable);
    disconnect(ringEditor->btnAdd,    &QPushButton::clicked, ringEditor, &StandardTableEditor::onAddItemClicked);
    disconnect(ringEditor->btnRemove, &QPushButton::clicked, ringEditor, &StandardTableEditor::onRemoveItemClicked);

    // Layout
    QGridLayout *ringInputLayout = new QGridLayout;
    ringInputLayout->setColumnMinimumWidth(0, 200);
    ringInputLayout->setColumnStretch(0, 0);
    ringInputLayout->setColumnStretch(1, 1);
    ringInputLayout->addWidget(new QLabel("Source group: "), 0, 0);
    ringInputLayout->addWidget(new QLabel("Buffer distance (m): "), 1, 0);
    ringInputLayout->addWidget(new QLabel("Receptor spacing (m): "), 2, 0);
    ringInputLayout->addWidget(cboSourceGroup, 0, 1);
    ringInputLayout->addWidget(sbRingBuffer, 1, 1);
    ringInputLayout->addWidget(sbRingSpacing, 2, 1);

    QVBoxLayout *ringTableLayout = new QVBoxLayout;
    ringTableLayout->setContentsMargins(0, 0, 0, 0);
    ringTableLayout->addWidget(ringEditor);
    ringTableLayout->addSpacing(5);
    ringTableLayout->addWidget(ringTable);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(ringInputLayout, 0);
    mainLayout->addSpacing(5);
    mainLayout->addLayout(ringTableLayout, 1);

    setLayout(mainLayout);
}

void ReceptorRingTab::onAddRingClicked()
{
    QColor color = QColor(Qt::black);
    double buffer = sbRingBuffer->value();
    double spacing = sbRingSpacing->value();

    // Check if a ring with the current buffer distance already exists.
    QModelIndex start = ringModel->index(0, 1);
    QModelIndexList match = ringModel->match(start, Qt::DisplayRole, QVariant::fromValue(buffer), 1);
    if (!match.isEmpty()) {
        // Ring exists; update it.
        QModelIndex index = match.value(0);
        QStandardItem *spacingItem = ringModel->item(index.row(), 2);
        spacingItem->setData(spacing, Qt::DisplayRole);
    }
    else {
        // Add a new ring.
        int currentRow = ringModel->rowCount();
        addStandardItem(ringModel, currentRow, 0, color, true);
        addStandardItem(ringModel, currentRow, 1, buffer, false);
        addStandardItem(ringModel, currentRow, 2, spacing, false);
    }
}

void ReceptorRingTab::onRemoveRingClicked()
{
    ringTable->removeSelectedRows();
}

//-----------------------------------------------------------------------------
// ReceptorNodeTab
//-----------------------------------------------------------------------------

ReceptorNodeTab::ReceptorNodeTab(QStandardItemModel *model, QWidget *parent)
    : QWidget(parent), nodeModel(model)
{
    leNodeX = new DoubleLineEdit(-10000000, 10000000, 2);
    leNodeX->setValue(0);

    leNodeY = new DoubleLineEdit(-10000000, 10000000, 2);
    leNodeY->setValue(0);

    leNodeZ = new DoubleLineEdit(0, 10000, 2);
    leNodeZ->setValue(0);

    nodeTable = new StandardTableView;
    nodeTable->setModel(nodeModel);
    nodeTable->setMinimumWidth(400);
    nodeTable->setItemDelegateForColumn(0, new ColorPickerDelegate);
    nodeTable->setDoubleLineEditForColumn(1, -10000000, 10000000, 11, false);
    nodeTable->setDoubleLineEditForColumn(2, -10000000, 10000000, 11, false);
    nodeTable->setDoubleLineEditForColumn(3, 0, 10000, 8, false);
    nodeTable->horizontalHeader()->setStretchLastSection(false);
    nodeTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    nodeTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    nodeTable->setSelectionMode(QAbstractItemView::ContiguousSelection);

    nodeEditor = new StandardTableEditor(Qt::Horizontal);

    // Connections
    connect(nodeEditor->btnAdd,    &QPushButton::clicked, this, &ReceptorNodeTab::onAddNodeClicked);
    connect(nodeEditor->btnRemove, &QPushButton::clicked, this, &ReceptorNodeTab::onRemoveNodeClicked);
    nodeEditor->init(nodeTable);
    disconnect(nodeEditor->btnAdd,    &QPushButton::clicked, nodeEditor, &StandardTableEditor::onAddItemClicked);
    disconnect(nodeEditor->btnRemove, &QPushButton::clicked, nodeEditor, &StandardTableEditor::onRemoveItemClicked);

    // Layout
    QGridLayout *nodeInputLayout = new QGridLayout;
    nodeInputLayout->setColumnMinimumWidth(0, 200);
    nodeInputLayout->setColumnStretch(0, 0);
    nodeInputLayout->setColumnStretch(1, 1);
    nodeInputLayout->setColumnStretch(2, 1);
    nodeInputLayout->addWidget(new QLabel("Receptor coordinates (m): "), 0, 0);
    nodeInputLayout->addWidget(leNodeX, 0, 1);
    nodeInputLayout->addWidget(leNodeY, 0, 2);
    nodeInputLayout->addWidget(new QLabel("Terrain elevation (m): "), 1, 0);
    nodeInputLayout->addWidget(leNodeZ, 1, 1, 1, 2);

    QVBoxLayout *nodeTableLayout = new QVBoxLayout;
    nodeTableLayout->setContentsMargins(0, 0, 0, 0);
    nodeTableLayout->addWidget(nodeEditor);
    nodeTableLayout->addSpacing(5);
    nodeTableLayout->addWidget(nodeTable);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(nodeInputLayout, 0);
    mainLayout->addSpacing(5);
    mainLayout->addLayout(nodeTableLayout, 1);

    setLayout(mainLayout);
}

void ReceptorNodeTab::onAddNodeClicked()
{
    QColor color = QColor(Qt::red);

    // Insert item at back.
    int currentRow = nodeModel->rowCount();
    addStandardItem(nodeModel, currentRow, 0, color, true);
    addStandardItem(nodeModel, currentRow, 1, leNodeX->value(), true);
    addStandardItem(nodeModel, currentRow, 2, leNodeY->value(), true);
    addStandardItem(nodeModel, currentRow, 3, leNodeZ->value(), true);
}

void ReceptorNodeTab::onRemoveNodeClicked()
{
    nodeTable->removeSelectedRows();
}

//-----------------------------------------------------------------------------
// ReceptorGridTab
//-----------------------------------------------------------------------------

ReceptorGridTab::ReceptorGridTab(QStandardItemModel *model, QWidget *parent)
    : QWidget(parent), gridModel(model)
{
    leGridXInit = new DoubleLineEdit(-10000000, 10000000, 2);
    leGridYInit = new DoubleLineEdit(-10000000, 10000000, 2);
    sbGridXCount = new QSpinBox;
    sbGridYCount = new QSpinBox;
    sbGridXDelta = new QDoubleSpinBox;
    sbGridYDelta = new QDoubleSpinBox;

    sbGridXCount->setRange(1, 1000);
    sbGridYCount->setRange(1, 1000);
    sbGridXDelta->setRange(0.1, 100000);
    sbGridYDelta->setRange(0.1, 100000);
    sbGridXDelta->setDecimals(2);
    sbGridYDelta->setDecimals(2);

    leGridXInit->setValue(0);
    leGridYInit->setValue(0);
    sbGridXCount->setValue(10);
    sbGridYCount->setValue(10);
    sbGridXDelta->setValue(100);
    sbGridYDelta->setValue(100);

    gridTable = new StandardTableView;
    gridTable->setModel(gridModel);
    gridTable->setMinimumWidth(400);
    gridTable->setItemDelegateForColumn(0, new ColorPickerDelegate);
    gridTable->setItemDelegateForColumn(1, new PointDelegate);
    gridTable->setItemDelegateForColumn(2, new PointDelegate);
    gridTable->setItemDelegateForColumn(3, new PointDelegate);
    gridTable->horizontalHeader()->setStretchLastSection(false);
    gridTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    gridTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    gridTable->setSelectionMode(QAbstractItemView::ContiguousSelection);

    gridEditor = new StandardTableEditor(Qt::Horizontal);

    // Connections
    connect(gridEditor->btnAdd,    &QPushButton::clicked, this, &ReceptorGridTab::onAddGridClicked);
    connect(gridEditor->btnRemove, &QPushButton::clicked, this, &ReceptorGridTab::onRemoveGridClicked);
    gridEditor->init(gridTable);
    disconnect(gridEditor->btnAdd,    &QPushButton::clicked, gridEditor, &StandardTableEditor::onAddItemClicked);
    disconnect(gridEditor->btnRemove, &QPushButton::clicked, gridEditor, &StandardTableEditor::onRemoveItemClicked);

    // Layout
    QGridLayout *gridInputLayout = new QGridLayout;
    gridInputLayout->setColumnMinimumWidth(0, 200);
    gridInputLayout->setColumnStretch(0, 0);
    gridInputLayout->setColumnStretch(1, 1);
    gridInputLayout->setColumnStretch(2, 1);
    gridInputLayout->addWidget(new QLabel("Starting grid location (m): "), 0, 0);
    gridInputLayout->addWidget(new QLabel("Receptor count: "), 1, 0);
    gridInputLayout->addWidget(new QLabel("Receptor spacing (m): "), 2, 0);
    gridInputLayout->addWidget(leGridXInit, 0, 1);
    gridInputLayout->addWidget(leGridYInit, 0, 2);
    gridInputLayout->addWidget(sbGridXCount, 1, 1);
    gridInputLayout->addWidget(sbGridYCount, 1, 2);
    gridInputLayout->addWidget(sbGridXDelta, 2, 1);
    gridInputLayout->addWidget(sbGridYDelta, 2, 2);

    QVBoxLayout *gridTableLayout = new QVBoxLayout;
    gridTableLayout->setContentsMargins(0, 0, 0, 0);
    gridTableLayout->addWidget(gridEditor);
    gridTableLayout->addSpacing(5);
    gridTableLayout->addWidget(gridTable);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(gridInputLayout, 0);
    mainLayout->addSpacing(5);
    mainLayout->addLayout(gridTableLayout, 1);

    setLayout(mainLayout);
}

void ReceptorGridTab::onAddGridClicked()
{
    QColor color = QColor(Qt::blue);
    QPointF pInit = QPointF(leGridXInit->value(), leGridYInit->value());
    QPoint pCount = QPoint(sbGridXCount->value(), sbGridYCount->value());
    QPointF pDelta = QPointF(sbGridXDelta->value(), sbGridYDelta->value());

    // Insert item at back.
    int currentRow = gridModel->rowCount();
    addStandardItem(gridModel, currentRow, 0, color, true);
    addStandardItem(gridModel, currentRow, 1, pInit, false);
    addStandardItem(gridModel, currentRow, 2, pCount, false);
    addStandardItem(gridModel, currentRow, 3, pDelta, false);
}

void ReceptorGridTab::onRemoveGridClicked()
{
    gridTable->removeSelectedRows();
}

//-----------------------------------------------------------------------------
// ReceptorDialog
//-----------------------------------------------------------------------------

ReceptorDialog::ReceptorDialog(Scenario *s, SourceGroup *sg, QWidget *parent)
    : QDialog(parent), sPtr(s), sgPtr(sg), saved(false)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowIcon(QIcon(":/images/EditReceptors_32x.png"));
    setWindowTitle(QString::fromStdString(sgPtr->grpid));

    ringModel = new QStandardItemModel;
    ringModel->setColumnCount(3);
    ringModel->setHorizontalHeaderLabels(QStringList{"", "Distance (m)", "Spacing (m)"});

    nodeModel = new QStandardItemModel;
    nodeModel->setColumnCount(4);
    nodeModel->setHorizontalHeaderLabels(QStringList{"", "X (m)", "Y (m)", "Z (m)"});

    gridModel = new QStandardItemModel;
    gridModel->setColumnCount(4);
    gridModel->setHorizontalHeaderLabels(QStringList{"", "Origin", "Count", "Spacing (m)"});

    ringTab = new ReceptorRingTab(ringModel, this);
    nodeTab = new ReceptorNodeTab(nodeModel, this);
    gridTab = new ReceptorGridTab(gridModel, this);

    QTabWidget *tabWidget = new QTabWidget;
    tabWidget->addTab(ringTab, "Rings");
    tabWidget->addTab(nodeTab, "Discrete");
    tabWidget->addTab(gridTab, "Uniform Grid");

    // Receptor Count
    lblReceptorCount = new QLabel("0");

    // Plot
    plot = new StandardPlot(this);
    plot->setAutoReplot(false);
    plot->setPanZoomMode(true);
    plot->setRescaler(true);

	// Button Box
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Close);

    // Layout
    QVBoxLayout *paramsLayout = new QVBoxLayout;
    paramsLayout->addWidget(tabWidget, 1);

    QVBoxLayout *canvasLayout = new QVBoxLayout;
    canvasLayout->addWidget(plot);

    QHBoxLayout *statsLayout = new QHBoxLayout;
    statsLayout->addWidget(new QLabel("Receptor count:"));
    statsLayout->addWidget(lblReceptorCount);
    statsLayout->addStretch(1);

    BackgroundFrame *plotFrame = new BackgroundFrame;
    plotFrame->setMinimumHeight(600);
    plotFrame->setMinimumWidth(600);
    plotFrame->setLayout(canvasLayout);

    QVBoxLayout *plotLayout = new QVBoxLayout;
    plotLayout->addLayout(statsLayout);
    plotLayout->addWidget(plotFrame);

    // Set size policies for split layout.
    tabWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QHBoxLayout *split = new QHBoxLayout;
    split->addWidget(tabWidget, 0);
    split->addLayout(plotLayout, 1);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(split);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    installEventFilter(this);
    init();
}

void ReceptorDialog::init()
{
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ReceptorDialog::reject);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ReceptorDialog::accept);

    // Call load before connecting dataChanged signals.
    load();
    updatePlot();

    connect(ringModel, &QStandardItemModel::dataChanged, this, [=] {
        rings = ringData();
        updatePlot();
    });

    connect(ringModel, &QStandardItemModel::rowsRemoved, this, [=] {
        rings = ringData();
        updatePlot();
    });

    connect(nodeModel, &QStandardItemModel::dataChanged, this, [=] {
        nodes = nodeData();
        updatePlot();
    });

    connect(nodeModel, &QStandardItemModel::rowsRemoved, this, [=] {
        nodes = nodeData();
        updatePlot();
    });

    connect(gridModel, &QStandardItemModel::dataChanged, this, [=] {
        grids = gridData();
        updatePlot();
    });

    connect(gridModel, &QStandardItemModel::rowsRemoved, this, [=] {
        grids = gridData();
        updatePlot();
    });
}

void ReceptorDialog::save()
{
    sgPtr->rings.clear();
    sgPtr->nodes.clear();
    sgPtr->grids.clear();
    sgPtr->rings = ringData();
    sgPtr->nodes = nodeData();
    sgPtr->grids = gridData();

    saved = true;
}

void ReceptorDialog::load()
{
    // Reset ring table
    for (const auto& ring : sgPtr->rings)
    {
        int currentRow = ringModel->rowCount();
        ringModel->insertRow(currentRow);
        addStandardItem(ringModel, currentRow, 0, ring.color, true);
        addStandardItem(ringModel, currentRow, 1, ring.buffer, false);
        addStandardItem(ringModel, currentRow, 2, ring.spacing, false);
    }

    // Reset node table
    for (const auto& node : sgPtr->nodes)
    {
        int currentRow = nodeModel->rowCount();
        nodeModel->insertRow(currentRow);
        addStandardItem(nodeModel, currentRow, 0, node.color, true);
        addStandardItem(nodeModel, currentRow, 1, node.x, false);
        addStandardItem(nodeModel, currentRow, 2, node.y, false);
        addStandardItem(nodeModel, currentRow, 3, node.zElev, false);
    }

    // Reset grid table
    for (const auto& grid : sgPtr->grids)
    {
        int currentRow = gridModel->rowCount();
        gridModel->insertRow(currentRow);
        QPointF pInit = QPointF(grid.xInit, grid.yInit);
        QPointF pCount = QPoint(grid.xCount, grid.yCount);
        QPointF pDelta = QPointF(grid.xDelta, grid.yDelta);
        addStandardItem(gridModel, currentRow, 0, grid.color, true);
        addStandardItem(gridModel, currentRow, 1, pInit, false);
        addStandardItem(gridModel, currentRow, 2, pCount, false);
        addStandardItem(gridModel, currentRow, 3, pDelta, false);
    }

    rings = ringData();
    nodes = nodeData();
    grids = gridData();
}

std::vector<ReceptorRing> ReceptorDialog::ringData() const
{
    std::vector<ReceptorRing> res;
    res.reserve(ringModel->rowCount());

    for (int row = 0; row < ringModel->rowCount(); ++row) {
        ReceptorRing ring;
        ring.color = ringModel->index(row, 0).data().value<QColor>();
        ring.buffer = ringModel->index(row, 1).data().value<double>();
        ring.spacing = ringModel->index(row, 2).data().value<double>();
        setRingGeometry(ring);

        res.push_back(ring);
    }

    return res;
}

std::vector<ReceptorNode> ReceptorDialog::nodeData() const
{
    std::vector<ReceptorNode> res;
    res.reserve(nodeModel->rowCount());

    for (int row = 0; row < nodeModel->rowCount(); ++row) {
        ReceptorNode node;
        node.color = nodeModel->index(row, 0).data().value<QColor>();
        node.x = nodeModel->index(row, 1).data().value<double>();
        node.y = nodeModel->index(row, 2).data().value<double>();
        double z = nodeModel->index(row, 3).data().value<double>();
        node.zElev = z;
        node.zHill = z;
        node.point = QPointF(node.x, node.y);

        res.push_back(node);
    }

    return res;
}

std::vector<ReceptorGrid> ReceptorDialog::gridData() const
{
    std::vector<ReceptorGrid> res;
    res.reserve(gridModel->rowCount());

    for (int row = 0; row < gridModel->rowCount(); ++row) {
        ReceptorGrid grid;
        grid.color = gridModel->index(row, 0).data().value<QColor>();
        grid.xInit = gridModel->index(row, 1).data().value<QPointF>().x();
        grid.yInit = gridModel->index(row, 1).data().value<QPointF>().y();
        grid.xCount = gridModel->index(row, 2).data().value<QPoint>().x();
        grid.yCount = gridModel->index(row, 2).data().value<QPoint>().y();
        grid.xDelta = gridModel->index(row, 3).data().value<QPointF>().x();
        grid.yDelta = gridModel->index(row, 3).data().value<QPointF>().y();
        setGridGeometry(grid);

        res.push_back(grid);
    }

    return res;
}

void ReceptorDialog::setRingGeometry(ReceptorRing &ring) const
{
    ring.points.clear();

    // Create the source mpolygon for the buffer.
    std::vector<QPolygonF> mpolygon;
    // FIXME: select the active source group for the ring
    for (const Source &s : sgPtr->sources)
        mpolygon.push_back(s.geometry);

    // Calculate points in circle using spacing as chord.
    double thetaR = 2.0 * std::asin(ring.spacing / (2.0 * ring.buffer));
    double thetaD = thetaR * (180.0 / M_PI);
    int npc = (std::max)(static_cast<int>(ceil(360.0 / thetaD)), 4);

    if (ring.spacing == 0)
        return;

    // Calculate the buffer and generate receptors along ring.
    GeosOp geosOp;
    std::vector<QPolygonF> merged = geosOp.unionCascaded(mpolygon);
    std::vector<QPolygonF> buffer = geosOp.buffer(merged, ring.buffer, npc);
    std::vector<QPolygonF> points = geosOp.measurePoints(buffer, ring.spacing);

    // Set geometry.
    for (const QPolygonF &p : buffer) {
        ring.polygons.push_back(p);
    }

    for (const QPolygonF &p : points) {
        for (const QPointF &pt : p) {
            ring.points.push_back(pt);
        }
    }
}

void ReceptorDialog::setGridGeometry(ReceptorGrid &grid) const
{
    grid.points.clear();

    for (int i=0; i < grid.xCount; ++i) {
        for (int j=0; j < grid.yCount; ++j) {
            double x = grid.xInit + (grid.xDelta * i);
            double y = grid.yInit + (grid.yDelta * j);
            grid.points.push_back(QPointF(x, y));
        }
    }
}

void ReceptorDialog::accept()
{
    save();
}

void ReceptorDialog::reject()
{
    if (saved)
        QDialog::done(QDialog::Accepted);
    else
        QDialog::done(QDialog::Rejected);
}

void ReceptorDialog::updatePlot()
{
    // Set busy status indicators
    this->setCursor(Qt::WaitCursor);

    plot->clear();
    int pointCount = 0;

    // Pen and brush for sources
    QPen pen(Qt::black, 1);
    pen.setJoinStyle(Qt::MiterJoin);
    QColor brush = QColor(Qt::green).lighter(175); // FIXME
    brush.setAlpha(200); // FIXME

    for (const Source &s : sgPtr->sources)
        plot->addRing(s.geometry, pen, brush);

    for (const auto& ring : rings) {
        for (const QPolygonF& polygon : ring.polygons) {
            plot->addRing(polygon);
        }
        plot->addPoints(ring.points, 5, ring.color);
        pointCount += ring.points.size();
    }

    for (const auto& node : nodes) {
        plot->addPoint(node.point, 5, node.color);
        pointCount += 1;
    }

    for (const auto& grid : grids) {
        plot->addPoints(grid.points, 5, grid.color);
        pointCount += grid.points.size();
    }

    lblReceptorCount->setText(QString::number(pointCount));

    plot->autoScale();
    plot->replot();

    // Set normal status indicators
    this->setCursor(Qt::ArrowCursor);
}

bool ReceptorDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* ke = static_cast<QKeyEvent*>(event);
        // Ignore escape, return keys
        if (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return ||
            ke->key() == Qt::Key_Escape) {
            return true;
        }
        return QObject::eventFilter(obj, event);
    }
    else {
        return QObject::eventFilter(obj, event);
    }
    return false;
}
