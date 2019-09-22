#include <QAbstractButton>
#include <QApplication>
#include <QBoxLayout>
#include <QButtonGroup>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QRegExp>
#include <QRegExpValidator>
#include <QSpinBox>
#include <QSplitter>
#include <QStackedLayout>
#include <QStackedWidget>
#include <QStylePainter>

#include <QDebug>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_shapeitem.h>
#include <qwt_symbol.h>

#include <algorithm>
#include <numeric>
#include <utility>

#include "ReceptorEditor.h"
#include "ReceptorVisitor.h"
#include "SourceGroup.h"
#include "widgets/GridLineFrame.h"
#include "widgets/NavButton.h"

//-----------------------------------------------------------------------------
// GroupEditor
//-----------------------------------------------------------------------------

GroupEditor::GroupEditor(ReceptorTreeView *view, QWidget *parent)
    : QWidget(parent), view(view)
{
    leGroupName = new QLineEdit;
    leGroupName->setMaxLength(8);
    QFont font = leGroupName->font();
    font.setCapitalization(QFont::AllUppercase);
    leGroupName->setFont(font);
    QRegExp rx("[A-Za-z0-9]+");
    QRegExpValidator *validator = new QRegExpValidator(rx, this);
    leGroupName->setValidator(validator);

    btnNode = new QRadioButton(tr("Discrete"));
    btnRing = new QRadioButton(tr("Ring"));
    btnGrid = new QRadioButton(tr("Grid"));

    btnAdd = new QPushButton;
    btnAdd->setText(tr("Add"));
    btnAdd->setAutoDefault(true);

    optionGroup = new QButtonGroup;
    optionGroup->addButton(btnNode, 0);
    optionGroup->addButton(btnRing, 1);
    optionGroup->addButton(btnGrid, 2);

    QHBoxLayout *optionLayout = new QHBoxLayout;
    optionLayout->setContentsMargins(0, 0, 0, 0);
    optionLayout->addWidget(btnNode);
    optionLayout->addWidget(btnRing);
    optionLayout->addWidget(btnGrid);
    optionLayout->addStretch(1);

    QWidget *options = new QWidget;
    options->setLayout(optionLayout);

    QFormLayout *inputLayout = new QFormLayout;
    inputLayout->setHorizontalSpacing(20);
    inputLayout->setVerticalSpacing(10);
    inputLayout->addRow(tr("Name:"), leGroupName);
    inputLayout->addRow(tr("Type:"), options);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(btnAdd);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(24, 16, 24, 16);
    mainLayout->addLayout(inputLayout);
    mainLayout->addStretch(1);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    btnAdd->setDisabled(true);

    connect(leGroupName, &QLineEdit::textChanged,
        [=](const QString&) { onEdit(); });

    connect(optionGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
        [=](int) { onEdit(); });

    connect(btnAdd, &QPushButton::clicked, this, &GroupEditor::onAddClicked);
}

void GroupEditor::setDefault(bool on)
{
    if (on)
        btnAdd->setDefault(true);
}

void GroupEditor::onEdit()
{
    bool validName = !leGroupName->text().isEmpty();
    bool validType = (optionGroup->checkedId() >= 0);

    if (validName & validType)
        btnAdd->setEnabled(true);
    else
        btnAdd->setDisabled(true);
}

void GroupEditor::onAddClicked(bool)
{
    ReceptorModel *model = qobject_cast<ReceptorModel *>(view->model());
    if (!model)
        return;

    QString grpid = leGroupName->text().toUpper();

    switch (optionGroup->checkedId()) {
    case 0: {
        auto g = ReceptorNodeGroup();
        g.grpid = grpid.toStdString();
        model->addGroup(g);
        break;
    }
    case 1: {
        auto g = ReceptorRingGroup();
        g.grpid = grpid.toStdString();
        model->addGroup(g);
        break;
    }
    case 2: {
        auto g = ReceptorGridGroup();
        g.grpid = grpid.toStdString();
        model->addGroup(g);
        break;
    }
    default:
        return;
    }

    view->selectLastRow();
}

//-----------------------------------------------------------------------------
// NodeEditor
//-----------------------------------------------------------------------------

NodeEditor::NodeEditor(QWidget *parent)
    : QWidget(parent)
{
    leNodeX = new DoubleLineEdit(-10000000, 10000000, 2);
    leNodeY = new DoubleLineEdit(-10000000, 10000000, 2);
    leNodeZ = new DoubleLineEdit(0, 9999.99, 2);
    leNodeZHill = new DoubleLineEdit(0, 9999.99, 2);
    leNodeZFlag = new DoubleLineEdit(0, 9999.99, 2);

    QHBoxLayout *xyzLayout = new QHBoxLayout;
    xyzLayout->setContentsMargins(0, 0, 0, 0);
    xyzLayout->addWidget(leNodeX);
    xyzLayout->addWidget(leNodeY);
    xyzLayout->addWidget(leNodeZ);

    QWidget *xyzWidget = new QWidget;
    xyzWidget->setLayout(xyzLayout);

    QFormLayout *inputLayout = new QFormLayout;
    inputLayout->setContentsMargins(16, 0, 0, 0);
    inputLayout->setHorizontalSpacing(20);
    inputLayout->setVerticalSpacing(5);
    inputLayout->addRow(tr("Coordinates:"), xyzWidget);
    inputLayout->addRow(tr("Hill Height:"), leNodeZHill);
    inputLayout->addRow(tr("Flagpole Height:"), leNodeZFlag);

    setLayout(inputLayout);
}

//-----------------------------------------------------------------------------
// RingParamsEditor
//-----------------------------------------------------------------------------

RingParamsEditor::RingParamsEditor(QWidget *parent)
    : QWidget(parent)
{
    cboSourceGroup = new QComboBox;

    sbBuffer = new QDoubleSpinBox;
    sbBuffer->setRange(0.1, 99000); // 99km
    sbBuffer->setValue(100);
    sbBuffer->setDecimals(2);

    sbSpacing = new QDoubleSpinBox;
    sbSpacing->setRange(0.1, 99000); // 99km
    sbSpacing->setValue(100);
    sbSpacing->setDecimals(2);

    QFormLayout *inputLayout = new QFormLayout;
    inputLayout->setContentsMargins(16, 0, 0, 0);
    inputLayout->setHorizontalSpacing(20);
    inputLayout->setVerticalSpacing(5);
    inputLayout->addRow(tr("Source group:"), cboSourceGroup);
    inputLayout->addRow(tr("Buffer distance:"), sbBuffer);
    inputLayout->addRow(tr("Receptor spacing:"), sbSpacing);

    setLayout(inputLayout);
}

//-----------------------------------------------------------------------------
// GridParamsEditor
//-----------------------------------------------------------------------------

GridParamsEditor::GridParamsEditor(QWidget *parent)
    : QWidget(parent)
{
    leGridXInit = new DoubleLineEdit(-10000000, 10000000, 2);
    leGridYInit = new DoubleLineEdit(-10000000, 10000000, 2);
    sbGridXCount = new QSpinBox;
    sbGridYCount = new QSpinBox;
    sbGridXDelta = new QDoubleSpinBox;
    sbGridYDelta = new QDoubleSpinBox;

    sbGridXCount->setRange(0, 1000);
    sbGridYCount->setRange(0, 1000);
    sbGridXDelta->setRange(0.1, 100000);
    sbGridYDelta->setRange(0.1, 100000);
    sbGridXDelta->setDecimals(2);
    sbGridYDelta->setDecimals(2);

    leGridXInit->setValue(0);
    leGridYInit->setValue(0);
    sbGridXCount->setValue(0);
    sbGridYCount->setValue(0);
    sbGridXDelta->setValue(100);
    sbGridYDelta->setValue(100);

    QHBoxLayout *xyLayout1 = new QHBoxLayout;
    xyLayout1->addWidget(leGridXInit);
    xyLayout1->addSpacing(5);
    xyLayout1->addWidget(leGridYInit);

    QHBoxLayout *xyLayout2 = new QHBoxLayout;
    xyLayout2->addWidget(sbGridXCount);
    xyLayout2->addSpacing(5);
    xyLayout2->addWidget(sbGridYCount);

    QHBoxLayout *xyLayout3 = new QHBoxLayout;
    xyLayout3->addWidget(sbGridXDelta);
    xyLayout3->addSpacing(5);
    xyLayout3->addWidget(sbGridYDelta);

    QFormLayout *inputLayout = new QFormLayout;
    inputLayout->setContentsMargins(16, 0, 0, 0);
    inputLayout->setHorizontalSpacing(20);
    inputLayout->setVerticalSpacing(5);
    inputLayout->addRow(tr("Grid origin:"), xyLayout1);
    inputLayout->addRow(tr("Receptor count:"), xyLayout2);
    inputLayout->addRow(tr("Receptor spacing:"), xyLayout3);

    setLayout(inputLayout);
}

//-----------------------------------------------------------------------------
// ReceptorParamsEditor
//-----------------------------------------------------------------------------

ReceptorParamsEditor::ReceptorParamsEditor(ReceptorTreeView *view, QWidget *parent)
    : QWidget(parent), view(view)
{
    headerLabel = new QLabel;
    QFont bold = headerLabel->font();
    bold.setBold(true);
    headerLabel->setFont(bold);

    statusLabel = new QLabel;
    statusLabel->setText("No items selected.");
    statusLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBaseline);

    nodeEditor = new NodeEditor;
    ringEditor = new RingParamsEditor;
    gridEditor = new GridParamsEditor;

    editorStack = new QStackedWidget;
    editorStack->addWidget(statusLabel);
    editorStack->addWidget(nodeEditor);
    editorStack->addWidget(ringEditor);
    editorStack->addWidget(gridEditor);
    editorStack->setCurrentIndex(0);

    btnAdd = new QPushButton(tr("Add"));
    btnRemove = new QPushButton(tr("Remove"));
    btnUpdate = new QPushButton(tr("Update"));

    btnAdd->setAutoDefault(true);
    btnRemove->setAutoDefault(false);
    btnUpdate->setAutoDefault(true);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(btnAdd);
    buttonLayout->addWidget(btnUpdate);
    buttonLayout->addWidget(btnRemove);

    QWidget *buttonContainer = new QWidget;
    buttonContainer->setLayout(buttonLayout);

    buttonStack = new QStackedWidget;
    buttonStack->addWidget(new QWidget);
    buttonStack->addWidget(buttonContainer);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(24, 16, 24, 16);
    mainLayout->addWidget(headerLabel);
    mainLayout->addWidget(editorStack);
    mainLayout->addSpacing(5);
    mainLayout->addStretch(1);
    mainLayout->addWidget(buttonStack);

    setLayout(mainLayout);

    editorStack->setCurrentIndex(0);
    buttonStack->setCurrentIndex(0);

    setupConnections();
}

ReceptorParamsEditor::~ReceptorParamsEditor()
{}

void ReceptorParamsEditor::load(Scenario *s)
{
    ringEditor->cboSourceGroup->clear();
    for (auto sgPtr : s->sourceGroups) {
        sgPtrs.emplace_back(std::weak_ptr(sgPtr));
        QString sgid = QString::fromStdString(sgPtr->grpid);
        ringEditor->cboSourceGroup->addItem(sgid);
    }
}

void ReceptorParamsEditor::setDefault(bool on)
{
    if (on) {
        if (btnAdd->isVisible()) {
            btnAdd->setDefault(true);
        }
        else {
            btnUpdate->setDefault(true);
        }
    }
}

void ReceptorParamsEditor::setupConnections()
{
    connect(btnRemove, &QPushButton::clicked,
            this, &ReceptorParamsEditor::onRemoveClicked);

    connect(btnAdd, &QPushButton::clicked,
            this, &ReceptorParamsEditor::onAddClicked);

    connect(btnUpdate, &QPushButton::clicked,
            this, &ReceptorParamsEditor::onUpdateClicked);

    connect(view->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ReceptorParamsEditor::onSelectionChanged);

    connect(view->model(), &QAbstractItemModel::dataChanged,
            this, &ReceptorParamsEditor::onDataChanged);
}

struct ReceptorParamsVisitor
    : public boost::static_visitor<>
{
    void operator()(const ReceptorNodeGroup& group) {
        return;
    }
    void operator()(const ReceptorRingGroup& group) {
        RingParamsEditor *re = editor->ringEditor;
        re->sbBuffer->setValue(group.buffer);
        re->sbSpacing->setValue(group.spacing);
        const std::weak_ptr<SourceGroup> wptr = group.sgPtr;
        if (wptr.expired())
            return;
        const auto& begin = editor->sgPtrs.cbegin();
        const auto& end = editor->sgPtrs.cend();
        auto compare = [&wptr](const std::weak_ptr<SourceGroup>& x) {
            return !(wptr.owner_before(x)) && !(x.owner_before(wptr));
        };
        auto it = std::find_if(begin, end, compare);
        int index = (it != end) ? std::distance(begin, it) : -1;
        re->cboSourceGroup->setCurrentIndex(index);
    }
    void operator()(const ReceptorGridGroup& group) {
        GridParamsEditor *ge = editor->gridEditor;
        ge->leGridXInit->setValue(group.xInit);
        ge->leGridYInit->setValue(group.yInit);
        ge->sbGridXCount->setValue(group.xCount);
        ge->sbGridYCount->setValue(group.yCount);
        ge->sbGridXDelta->setValue(group.xDelta);
        ge->sbGridYDelta->setValue(group.yDelta);
    }

    ReceptorParamsEditor *editor;
};

void ReceptorParamsEditor::resetControls()
{
    headerLabel->clear();
    statusLabel->clear();
    btnAdd->setVisible(true);
    btnRemove->setVisible(true);
    btnUpdate->setVisible(true);
    nodeEditor->leNodeX->clear();
    nodeEditor->leNodeY->clear();
    nodeEditor->leNodeZ->clear();
    nodeEditor->leNodeZHill->clear();
    nodeEditor->leNodeZFlag->clear();

    // Set current editor.
    switch (currentState) {
    case StateFlag::NoSelection:
        statusLabel->setText("No items selected.");
        editorStack->setCurrentIndex(0);
        buttonStack->setCurrentIndex(0);
        return;
    case StateFlag::InvalidSelection:
        statusLabel->setText("Multiple item types selected.");
        editorStack->setCurrentIndex(0);
        buttonStack->setCurrentIndex(0);
        return;
    case StateFlag::EditRingGroup:
        editorStack->setCurrentIndex(2);
        buttonStack->setCurrentIndex(1);
        break;
    case StateFlag::EditGridGroup:
        editorStack->setCurrentIndex(3);
        buttonStack->setCurrentIndex(1);
        break;
    default:
        editorStack->setCurrentIndex(1);
        buttonStack->setCurrentIndex(1);
        break;
    }

    // Update editor controls.
    switch (currentState) {
    case StateFlag::EditDiscreteGroup:
        nodeEditor->leNodeX->setEnabled(true);
        nodeEditor->leNodeY->setEnabled(true);
        nodeEditor->leNodeX->setValue(0);
        nodeEditor->leNodeY->setValue(0);
        nodeEditor->leNodeZ->setValue(0);
        nodeEditor->leNodeZHill->setValue(0);
        nodeEditor->leNodeZFlag->setValue(0);
        btnRemove->setText(tr("Remove Group"));
        btnUpdate->setVisible(false);
        break;
    case StateFlag::EditRingGroup:
        btnAdd->setVisible(false);
        btnRemove->setText(tr("Remove Group"));
        break;
    case StateFlag::EditGridGroup:
        btnAdd->setVisible(false);
        btnRemove->setText(tr("Remove Group"));
        break;
    case StateFlag::EditSingleDiscrete:
        // TODO: Allow editing all fields
        nodeEditor->leNodeX->setEnabled(false);
        nodeEditor->leNodeY->setEnabled(false);
        btnAdd->setVisible(false);
        btnRemove->setText(tr("Remove"));
        break;
    case StateFlag::EditMultipleDiscrete:
        nodeEditor->leNodeX->setEnabled(false);
        nodeEditor->leNodeY->setEnabled(false);
        btnAdd->setVisible(false);
        btnRemove->setText(tr("Remove"));
        break;
    case StateFlag::EditSingleRing:
    case StateFlag::EditSingleGrid:
    case StateFlag::EditMultipleRing:
    case StateFlag::EditMultipleGrid:
        nodeEditor->leNodeX->setEnabled(false);
        nodeEditor->leNodeY->setEnabled(false);
        btnAdd->setVisible(false);
        btnRemove->setVisible(false);
        break;
    default:
        return;
    }

    if (btnAdd->isDefault() || btnUpdate->isDefault())
        setDefault(true);

    ReceptorModel *model = qobject_cast<ReceptorModel *>(view->model());
    QModelIndex& index = selectedRows.front();
    const ReceptorGroup& group = model->groupFromIndex(index);

    switch (currentState) {
    case StateFlag::EditSingleDiscrete:
    case StateFlag::EditSingleRing:
    case StateFlag::EditSingleGrid:
        const QModelIndex ix = index.siblingAtColumn(ReceptorModel::Column::X);
        const QModelIndex iy = index.siblingAtColumn(ReceptorModel::Column::Y);
        const QModelIndex iz = index.siblingAtColumn(ReceptorModel::Column::Z);
        const QModelIndex izhill = index.siblingAtColumn(ReceptorModel::Column::ZHill);
        const QModelIndex izflag = index.siblingAtColumn(ReceptorModel::Column::ZFlag);
        nodeEditor->leNodeX->setValue(model->data(ix, Qt::DisplayRole).toDouble());
        nodeEditor->leNodeY->setValue(model->data(iy, Qt::DisplayRole).toDouble());
        nodeEditor->leNodeZ->setValue(model->data(iz, Qt::DisplayRole).toDouble());
        nodeEditor->leNodeZHill->setValue(model->data(izhill, Qt::DisplayRole).toDouble());
        nodeEditor->leNodeZFlag->setValue(model->data(izflag, Qt::DisplayRole).toDouble());
        break;
    }

    QString grpid = QString::fromStdString(boost::apply_visitor(ReceptorGroupNameVisitor(), group));
    resetHeader(grpid);

    ReceptorParamsVisitor visitor;
    visitor.editor = this;
    boost::apply_visitor(visitor, group);
}

void ReceptorParamsEditor::resetHeader(const QString& grpid)
{
    QString header = grpid;

    switch (currentState) {
    case StateFlag::EditDiscreteGroup:
        header.append(": Add Receptor");
        break;
    case StateFlag::EditRingGroup:
        header.append(": Edit Ring");
        break;
    case StateFlag::EditGridGroup:
        header.append(": Edit Grid");
        break;
    case StateFlag::EditSingleDiscrete:
    case StateFlag::EditSingleRing:
    case StateFlag::EditSingleGrid:
        header.append(": Edit Receptor");
        break;
    case StateFlag::EditMultipleDiscrete:
    case StateFlag::EditMultipleRing:
    case StateFlag::EditMultipleGrid:
        header.append(": Edit Multiple Receptors");
        break;
    default:
        break;
    }

    headerLabel->setText(header);
}

void ReceptorParamsEditor::onRemoveClicked(bool)
{
    if (selectedRows.size() == 0)
        return;

    QAbstractItemModel *model = view->model();

    // Sort the selection in ascending order.
    std::sort(selectedRows.begin(), selectedRows.end(),
        [](const auto& a, const auto& b) {
            return a.row() > b.row();
        });

    // Partition selected rows into contiguous ranges.
    // Assumes that all rows have the same parent.
    std::vector<std::pair<int, int>> remove;
    auto current = selectedRows.begin();
    while (current != selectedRows.end()) {
        auto next = std::adjacent_find(current, selectedRows.end(),
            [](const auto& a, const auto& b) {
                return (a.row() - b.row() > 1);
            });

        if (next != selectedRows.end()) {
            auto start = next;
            int count = std::distance(current, std::next(next));
            remove.emplace_back(std::make_pair(start->row(), count));
            current = std::next(next);
        }
        else {
            auto start = std::prev(next);
            int count = std::distance(current, next);
            remove.emplace_back(std::make_pair(start->row(), count));
            break;
        }
    }

    QModelIndex parent = selectedRows.front().parent();
    for (const auto& p : remove) {
        model->removeRows(p.first, p.second, parent);
    }

    selectedRows.clear();
    view->selectionModel()->clearSelection();
}

void ReceptorParamsEditor::onAddClicked(bool)
{
    if (selectedRows.size() == 0)
        return;

    ReceptorModel *model = qobject_cast<ReceptorModel *>(view->model());
    if (!model)
        return;

    QModelIndex& index = selectedRows.front();

    ReceptorNode node;
    node.x = nodeEditor->leNodeX->value();
    node.y = nodeEditor->leNodeY->value();
    node.zElev = nodeEditor->leNodeZ->value();
    node.zHill = nodeEditor->leNodeZHill->value();
    node.zFlag = nodeEditor->leNodeZFlag->value();

    model->addReceptor(index, node);
}

void ReceptorParamsEditor::onUpdateClicked(bool)
{
    if (selectedRows.size() == 0)
        return;

    ReceptorModel *model = qobject_cast<ReceptorModel *>(view->model());
    if (!model)
        return;

    QModelIndex& index = selectedRows.front();
    if (selectedRows.size() == 1 && index.internalId() == 0) {
        // Updating a ring or grid.
        if (currentState == StateFlag::EditRingGroup) {
            RingParamsEditor *re = this->ringEditor;
            double buffer = re->sbBuffer->value();
            double spacing = re->sbSpacing->value();
            int sgi = re->cboSourceGroup->currentIndex();
            if (sgi < 0 || sgi > sgPtrs.size())
                return;
            std::shared_ptr<SourceGroup> sgPtr = sgPtrs.at(sgi).lock();
            if (!sgPtr)
                return;
            model->updateRing(index, buffer, spacing, sgPtr);
        }
        else if (currentState == StateFlag::EditGridGroup) {
            GridParamsEditor *ge = this->gridEditor;
            double xInit = ge->leGridXInit->value();
            double yInit = ge->leGridYInit->value();
            int xCount = ge->sbGridXCount->value();
            int yCount = ge->sbGridYCount->value();
            double xDelta = ge->sbGridXDelta->value();
            double yDelta = ge->sbGridYDelta->value();
            model->updateGrid(index, xInit, yInit, xCount, yCount, xDelta, yDelta);
        }
    }
    else {
        // Updating elevations for one or more receptors.
        bool zElevChanged = !nodeEditor->leNodeZ->text().isEmpty();
        bool zHillChanged = !nodeEditor->leNodeZHill->text().isEmpty();
        bool zFlagChanged = !nodeEditor->leNodeZFlag->text().isEmpty();

        double zElev = nodeEditor->leNodeZ->value();
        double zHill = nodeEditor->leNodeZHill->value();
        double zFlag = nodeEditor->leNodeZFlag->value();

        for (const QModelIndex& index : selectedRows) {
            if (zElevChanged)
                model->updateZElev(index, zElev);
            if (zHillChanged)
                model->updateZHill(index, zHill);
            if (zFlagChanged)
                model->updateZFlag(index, zFlag);
        }
    }
}

void ReceptorParamsEditor::onDataChanged(const QModelIndex& first, const QModelIndex& last, const QVector<int>&)
{
    // Update header labels when the group name is changed in the table.
    if (first.row() != last.row())
        return;

    if (first.internalId() != 0)
        return;

    ReceptorModel *model = qobject_cast<ReceptorModel *>(view->model());
    const ReceptorGroup& group = model->groupFromIndex(first);
    QString grpid = QString::fromStdString(boost::apply_visitor(ReceptorGroupNameVisitor(), group));
    resetHeader(grpid);
}

void ReceptorParamsEditor::onSelectionChanged(const QItemSelection&, const QItemSelection&)
{
    selectedRows = view->selectionModel()->selectedRows();
    ReceptorModel *model = qobject_cast<ReceptorModel *>(view->model());
    ReceptorParamsEditor::State state;

    if (selectedRows.empty()) {
        currentState = StateFlag::NoSelection;
        resetControls();
        return;
    }

    QModelIndex& first = selectedRows.front();
    int id = first.internalId();

    if (selectedRows.size() != 1) {
        if (id == 0) {
            // Only one group can be selected.
            currentState = StateFlag::InvalidSelection;
            resetControls();
            return;
        }
        else {
            // If multiple receptors are selected, they must be in the same group.
            auto it = std::adjacent_find(selectedRows.cbegin(), selectedRows.cend(),
                [](const auto& a, const auto& b) {
                    return a.internalId() != b.internalId();
                });

            if (it != selectedRows.end()) {
                currentState = StateFlag::InvalidSelection;
                resetControls();
                return;
            }
        }
    }

    ReceptorGroup& group = model->groupFromIndex(first);
    int groupType = group.which();

    if (selectedRows.size() == 1 && id == 0) {
        // Selection has one group.
        switch (groupType) {
        case 0: currentState = StateFlag::EditDiscreteGroup;
            break;
        case 1: currentState = StateFlag::EditRingGroup;
            break;
        case 2: currentState = StateFlag::EditGridGroup;
            break;
        default:
            break;
        }
    }
    else if (selectedRows.size() == 1 && id > 0) {
        // Selection has one receptor.
        switch (groupType) {
        case 0: currentState = StateFlag::EditSingleDiscrete;
            break;
        case 1: currentState = StateFlag::EditSingleRing;
            break;
        case 2: currentState = StateFlag::EditSingleGrid;
            break;
        default:
            break;
        }
    }
    else {
        // Selection has multiple receptors.
        switch (groupType) {
        case 0: currentState = StateFlag::EditMultipleDiscrete;
            break;
        case 1: currentState = StateFlag::EditMultipleRing;
            break;
        case 2: currentState = StateFlag::EditMultipleGrid;
            break;
        default:
            break;
        }
    }

    resetControls();
}

//-----------------------------------------------------------------------------
// ReceptorEditor
//-----------------------------------------------------------------------------

ReceptorEditor::ReceptorEditor(QWidget *parent)
    : QWidget(parent)
{
    model = new ReceptorModel(this);
    view = new ReceptorTreeView(model);
    groupEditor = new GroupEditor(view);
    paramsEditor = new ReceptorParamsEditor(view);

    // Navigation
    NavButton *navAdd = new NavButton(tr("Add Group"));
    NavButton *navEdit = new NavButton(tr("Edit Receptors"));

    QButtonGroup *navGroup = new QButtonGroup;
    navGroup->addButton(navAdd, 0);
    navGroup->addButton(navEdit, 1);

    plot = new StandardPlot(this);
    plot->setAutoReplot(false);
    plot->setAutoDelete(true);
    plot->setPanZoomMode(true);
    plot->setRescaler(true);
    plot->setItemTitleTracker(true);
    plot->setMinimumHeight(600);
    plot->setMinimumWidth(600);

    // Layout
    QHBoxLayout *navLayout = new QHBoxLayout;
    navLayout->setSpacing(0);
    navLayout->addWidget(navAdd);
    navLayout->addWidget(navEdit);

    QStackedLayout *navStackLayout = new QStackedLayout;
    navStackLayout->addWidget(groupEditor);
    navStackLayout->addWidget(paramsEditor);

    QVBoxLayout *viewLayout = new QVBoxLayout;
    viewLayout->setContentsMargins(0, 0, 0, 0);
    viewLayout->setSpacing(0);
    viewLayout->addWidget(new GridLineFrame(Qt::Horizontal), 0);
    viewLayout->addWidget(view, 1);

    QVBoxLayout *controlsLayout = new QVBoxLayout;
    controlsLayout->setContentsMargins(0, 0, 0, 0);
    controlsLayout->addLayout(navLayout, 0);
    controlsLayout->addLayout(navStackLayout, 0);
    controlsLayout->addLayout(viewLayout, 1);

    QWidget *controlsArea = new QWidget;
    controlsArea->setLayout(controlsLayout);

    QVBoxLayout *plotLayout = new QVBoxLayout;
    plotLayout->setContentsMargins(32, 32, 32, 32);
    plotLayout->addWidget(plot, 1);

    QHBoxLayout *plotFrameLayout = new QHBoxLayout;
    plotFrameLayout->setContentsMargins(0, 0, 0, 0);
    plotFrameLayout->setSpacing(0);
    plotFrameLayout->addWidget(new GridLineFrame(Qt::Vertical), 0);
    plotFrameLayout->addLayout(plotLayout, 1);

    QWidget *plotArea = new QWidget;
    plotArea->setLayout(plotFrameLayout);

    const int iconSize = QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize);
    QFontMetrics fm(QApplication::font());
    const int charWidth = fm.averageCharWidth();
    const int headerWidth = view->indentation() + iconSize * 1.5 + charWidth * 9 * 5;
    controlsArea->setMinimumWidth(headerWidth + 2);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setContentsMargins(0, 0, 0, 0);
    splitter->setHandleWidth(0);
    splitter->addWidget(controlsArea);
    splitter->addWidget(plotArea);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(splitter);

    QPalette palette = this->palette();
    QColor bgcolor = QWidget::palette().window().color();
    palette.setColor(QPalette::Window, bgcolor.lighter(106));
    setPalette(palette);
    setBackgroundRole(QPalette::Window);
    setAutoFillBackground(true);

    setLayout(mainLayout);

    connect(navGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
        [=](int id) {
            navStackLayout->setCurrentIndex(id);
            switch (id) {
            case 0: groupEditor->setDefault(true); break;
            case 1: paramsEditor->setDefault(true); break;
            default: break;
            }
        });

    navAdd->setChecked(true);

    setupConnections();
}

ReceptorEditor::~ReceptorEditor()
{}

void ReceptorEditor::setupConnections()
{
    connect(model, &QAbstractItemModel::modelReset,
            this, &ReceptorEditor::onModelReset);

    connect(model, &QAbstractItemModel::rowsInserted,
            this, &ReceptorEditor::onRowsInserted);

    connect(model, &QAbstractItemModel::rowsRemoved,
            this, &ReceptorEditor::onRowsRemoved);

    connect(model, &QAbstractItemModel::dataChanged,
            this, &ReceptorEditor::onDataChanged);
}

void ReceptorEditor::onModelReset()
{
    plotItems.clear();
    for (int i = 0; i < model->rowCount(); ++i) {
        QModelIndex parent = model->index(i, 0, QModelIndex());
        int n = model->rowCount(parent);
        onRowsInserted(parent, 0, n - 1);
    }
}

void ReceptorEditor::onRowsInserted(const QModelIndex& parent, int first, int last)
{
    // Create and attach plot items when receptors are added.
    // The key value is always the parent index.

    using PlotShapeItemPtr = std::unique_ptr<QwtPlotShapeItem, PlotItemDeleterFunctor>;
    using PlotCurvePtr = std::unique_ptr<QwtPlotCurve, PlotItemDeleterFunctor>;
    using PlotMarkerPtr = std::unique_ptr<QwtPlotMarker, PlotItemDeleterFunctor>;

    if (!parent.isValid() || parent.internalId() != 0)
        return;

    auto& variant = model->groupFromIndex(parent);

    if (variant.which() == 0) {
        ReceptorNodeGroup& group = boost::get<ReceptorNodeGroup>(variant);

        // Add markers for receptor nodes.
        for (int i = first; i <= last; ++i) {
            QwtSymbol *sym = new QwtSymbol;
            sym->setStyle(QwtSymbol::Ellipse);
            sym->setSize(5, 5);
            sym->setBrush(QBrush(group.color));

            ReceptorNode node = group.getNode(i);
            PlotMarkerPtr marker(new QwtPlotMarker());
            marker->setValue(node.point());
            marker->setSymbol(sym);
            marker->attach(plot);
            plotItems.emplace(std::make_pair(parent, std::move(marker)));
        }
    }
    else if (variant.which() == 1) {
        ReceptorRingGroup& group = boost::get<ReceptorRingGroup>(variant);

        // Add polygons for receptor rings.
        for (const QPolygonF& polygon : group.polygons) {
            PlotShapeItemPtr shape(new QwtPlotShapeItem());
            shape->setPolygon(polygon);
            shape->attach(plot);
            plotItems.emplace(std::make_pair(parent, std::move(shape)));
        }

        // Add curves for receptor rings.
        PlotCurvePtr curve(new QwtPlotCurve());
        curve->setStyle(QwtPlotCurve::Dots);
        QPen pen = QPen(QBrush(group.color), 5);
        curve->setPen(pen);
        curve->setSamples(group.points());
        curve->attach(plot);
        plotItems.emplace(std::make_pair(parent, std::move(curve)));
    }
    else if (variant.which() == 2) {
        ReceptorGridGroup& group = boost::get<ReceptorGridGroup>(variant);

        // Add curves for receptor grids.
        PlotCurvePtr curve(new QwtPlotCurve());
        curve->setStyle(QwtPlotCurve::Dots);
        QPen pen = QPen(QBrush(group.color), 5);
        curve->setPen(pen);
        curve->setSamples(group.points());
        curve->attach(plot);
        plotItems.emplace(std::make_pair(parent, std::move(curve)));
    }

    plot->replot();
}

void ReceptorEditor::onRowsRemoved(const QModelIndex& parent, int first, int last)
{
    if (first < 0 || last < 0)
        return;

    if (parent.isValid() && parent.internalId() == 0) {
        // Receptors removed; delete plot items in range, or all plot items
        // (including extra polygons for receptor rings) if row count is zero.
        if (model->rowCount(parent) == 0) {
            plotItems.erase(parent);
        }
        else {
            auto range = plotItems.equal_range(parent);
            auto it = std::next(range.first, first);
            auto end = std::next(range.first, first + last);
            plotItems.erase(it, end);
        }
    }
    else {
        // Receptor groups removed; delete all associated plot items.
        for (int i = first; i <= last; ++i) {
            QModelIndex index = model->index(i, 0, QModelIndex());
            if (plotItems.count(index))
                plotItems.erase(index);
        }
    }

    plot->replot();
}

void ReceptorEditor::onDataChanged(const QModelIndex& first, const QModelIndex& last, const QVector<int>&)
{
    // Update receptor colors. Only applies to groups.
    if (first.internalId() != 0 || last.internalId() != 0)
        return;

    if (first.column() != ReceptorModel::Column::Color)
        return;

    for (int i = first.row(); i <= last.row(); ++i) {
        QModelIndex index = model->index(i, 0, QModelIndex());
        QColor color = qvariant_cast<QColor>(model->data(index, Qt::DisplayRole));
        auto range = plotItems.equal_range(index);
        for (auto it = range.first; it != range.second; ++it) {
            int rtti = it->second->rtti();
            if (rtti == QwtPlotItem::Rtti_PlotCurve) {
                QwtPlotCurve *curve = static_cast<QwtPlotCurve *>(it->second.get());
                QPen pen = QPen(QBrush(color), 5);
                curve->setPen(pen);
            }
            else if (rtti == QwtPlotItem::Rtti_PlotMarker) {
                QwtPlotMarker *marker = static_cast<QwtPlotMarker *>(it->second.get());
                QwtSymbol *sym = new QwtSymbol();
                sym->setStyle(QwtSymbol::Ellipse);
                sym->setSize(5, 5);
                sym->setBrush(QBrush(color));
                marker->setSymbol(sym);
            }
        }
    }

    plot->replot();
}

void ReceptorEditor::save(Scenario *s)
{
    model->save(s->receptors);
}

void ReceptorEditor::load(Scenario *s)
{
    paramsEditor->load(s);
    model->load(s->receptors);

    for (const auto sgptr : s->sourceGroups) {
        for (const Source &s : sgptr->sources) {
            QString title = QString::fromStdString(s.srcid);
            plot->addRing(s.geometry, s.pen, s.brush, title);
        }
    }

    plot->autoScale();
    plot->replot();
}
