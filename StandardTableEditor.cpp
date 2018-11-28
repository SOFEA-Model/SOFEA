#include <QDir>
#include <QFileDialog>
#include <QSettings>

#include "StandardTableEditor.h"

StandardTableEditor::StandardTableEditor(Qt::Orientation orientation, StandardButtons buttons, QWidget *parent)
    : QWidget(parent)
{
    m_importFilter = nullptr;
    m_importCaption = tr("Import File");

    // Layout
    QBoxLayout *mainLayout;
    switch (orientation) {
    case Qt::Vertical:
        mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
        break;
    case Qt::Horizontal:
        mainLayout = new QBoxLayout(QBoxLayout::LeftToRight);
        break;
    default:
        return;
    }

    mainLayout->setMargin(0);

    if (orientation == Qt::Horizontal) {
        mainLayout->addStretch(1);
    }

    if (buttons.testFlag(Add)) {
        btnAdd = new QPushButton("Add");
        mainLayout->addWidget(btnAdd);
    }
    if (buttons.testFlag(Remove)) {
        btnRemove = new QPushButton("Remove");
        mainLayout->addWidget(btnRemove);
    }
    if (buttons.testFlag(Rename)) {
        btnRename = new QPushButton("Rename");
        mainLayout->addWidget(btnRename);
    }

    mainLayout->addSpacing(10);

    if (buttons.testFlag(MoveUp)) {
        btnMoveUp = new QPushButton("Move Up");
        mainLayout->addWidget(btnMoveUp);
    }
    if (buttons.testFlag(MoveDown)) {
        btnMoveDown = new QPushButton("Move Down");
        mainLayout->addWidget(btnMoveDown);
    }

    mainLayout->addSpacing(10);

    if (buttons.testFlag(Edit)) {
        btnEdit = new QPushButton("Edit...");
        mainLayout->addWidget(btnEdit);
    }
    if (buttons.testFlag(Import)) {
        btnImport = new QPushButton("Import...");
        mainLayout->addWidget(btnImport);
    }

    if (orientation == Qt::Vertical) {
        mainLayout->addStretch(1);
    }

    setLayout(mainLayout);
}

void StandardTableEditor::init(StandardTableView *standardTableView)
{
    m_standardTableView = standardTableView;

    if (m_standardTableView->selectionModel()->selectedIndexes().isEmpty()) {
        if (btnRemove)   btnRemove->setEnabled(false);
        if (btnMoveUp)   btnMoveUp->setEnabled(false);
        if (btnMoveDown) btnMoveDown->setEnabled(false);
    }

    if (m_standardTableView->selectionModel()->selectedRows().count() != 1) {
        if (btnEdit)     btnEdit->setEnabled(false);
        if (btnRename)   btnRename->setEnabled(false);
    }

    // Connections
    if (btnAdd)      connect(btnAdd,      &QPushButton::clicked, this, &StandardTableEditor::onAddItemClicked);
    if (btnRemove)   connect(btnRemove,   &QPushButton::clicked, this, &StandardTableEditor::onRemoveItemClicked);
    if (btnRename)   connect(btnRename,   &QPushButton::clicked, this, &StandardTableEditor::onRenameItemClicked);
    if (btnMoveUp)   connect(btnMoveUp,   &QPushButton::clicked, this, &StandardTableEditor::onMoveRequested);
    if (btnMoveDown) connect(btnMoveDown, &QPushButton::clicked, this, &StandardTableEditor::onMoveRequested);
    if (btnEdit)     connect(btnEdit,     &QPushButton::clicked, this, &StandardTableEditor::onEditItemClicked);
    if (btnImport)   connect(btnImport,   &QPushButton::clicked, this, &StandardTableEditor::onImportClicked);

    connect(m_standardTableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &StandardTableEditor::onSelectionChanged);
}

void StandardTableEditor::setImportFilter(const QString& filter)
{
    m_importFilter = filter;
}

void StandardTableEditor::setImportCaption(const QString& caption)
{
    m_importCaption = caption;
}

QString StandardTableEditor::importFile()
{
    return m_importFile;
}

void StandardTableEditor::onAddItemClicked()
{
    if (!m_standardTableView)
        return;

    m_standardTableView->addRow();
    m_standardTableView->selectLastRow();
    m_standardTableView->setFocus();
}

void StandardTableEditor::onRemoveItemClicked()
{
    if (!m_standardTableView)
        return;

    m_standardTableView->removeSelectedRows();
}

void StandardTableEditor::onRenameItemClicked()
{
    if (!m_standardTableView)
        return;

    QModelIndexList selectedRows = m_standardTableView->selectionModel()->selectedRows();
    if (selectedRows.count() == 1) {
        m_standardTableView->edit(selectedRows.first());
    }
}

void StandardTableEditor::onMoveRequested()
{
    if (!m_standardTableView)
        return;

    QPushButton *sender = qobject_cast<QPushButton *>(QObject::sender());
    if (sender != btnMoveUp && sender != btnMoveDown)
        return;

    QModelIndexList rows = m_standardTableView->selectionModel()->selectedRows();

    if (rows.empty())
        return;

    auto index = std::min_element(rows.begin(), rows.end(),
        [](const QModelIndex& a, const QModelIndex& b)->bool {
            return a.row() < b.row();
        });

    int sourceRow = index->row();

    int offset = sender == btnMoveUp ? -1 : 1;

    emit moveRequested(QModelIndex(), sourceRow, rows.size(),
                       QModelIndex(), sourceRow + offset);

    m_standardTableView->setFocus();
}

void StandardTableEditor::onEditItemClicked()
{
    if (!m_standardTableView)
        return;

    QModelIndexList selectedRows = m_standardTableView->selectionModel()->selectedRows();
    if (selectedRows.count() == 1) {
        emit editRequested(selectedRows.first());
    }
}

void StandardTableEditor::onImportClicked()
{
    if (!m_standardTableView)
        return;

    int cols = m_standardTableView->model()->columnCount();
    if (cols == 0)
        return;

    QSettings settings;
    QString currentDir = settings.value("DefaultDirectory", QDir::rootPath()).toString();
    QString file = QFileDialog::getOpenFileName(this,
                   m_importCaption,
                   currentDir,
                   m_importFilter);

    if (file.isEmpty())
        return;

    m_importFile = file;
    emit importRequested();
}

void StandardTableEditor::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    if (!m_standardTableView)
        return;

    bool hasSelection = m_standardTableView->selectionModel()->hasSelection();
    if (btnRemove) btnRemove->setEnabled(hasSelection);
    if (!hasSelection) {
        if (btnMoveUp)   btnMoveUp->setEnabled(false);
        if (btnMoveDown) btnMoveDown->setEnabled(false);
        return;
    }

    // Sort selection ascending.
    QModelIndexList selectedRows = m_standardTableView->selectionModel()->selectedRows();
    std::sort(selectedRows.begin(), selectedRows.end(),
        [](const QModelIndex& a, const QModelIndex& b)->bool {
        return a.row() < b.row();
    });

    bool singleSelection = selectedRows.count() == 1;
    if (btnEdit)   btnEdit->setEnabled(singleSelection);
    if (btnRename) btnRename->setEnabled(singleSelection);

    // Only enable move for contiguous selection modes.
    if (m_standardTableView->selectionMode() != QAbstractItemView::SingleSelection &&
        m_standardTableView->selectionMode() != QAbstractItemView::ContiguousSelection) {
        btnMoveUp->setEnabled(false);
        btnMoveDown->setEnabled(false);
        return;
    }

    // Check for valid move up.
    if (btnMoveUp) {
        int start = selectedRows.first().row();
        if (start - 1 < 0)
            btnMoveUp->setEnabled(false);
        else
            btnMoveUp->setEnabled(true);
    }

    // Check for valid move down.
    if (btnMoveDown) {
        int start = selectedRows.last().row();
        int nrows = m_standardTableView->model()->rowCount();
        if (start + 1 >= nrows)
            btnMoveDown->setEnabled(false);
        else
            btnMoveDown->setEnabled(true);
    }
}
