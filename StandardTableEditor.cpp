#include <QDir>
#include <QFileDialog>
#include <QSettings>

#include "StandardTableEditor.h"

StandardTableEditor::StandardTableEditor(const QBoxLayout::Direction direction, QWidget *parent) : QWidget(parent)
{
    btnAdd = new QPushButton("Add");
    btnRemove = new QPushButton("Remove");
    btnImport = new QPushButton("Import...");
    btnImport->setVisible(false);
    btnImport->setEnabled(false);

    m_importFilter = nullptr;
    m_importCaption = tr("Import File");

    QBoxLayout *mainLayout = new QBoxLayout(direction);
    mainLayout->setMargin(0);
    mainLayout->addWidget(btnAdd);
    mainLayout->addWidget(btnRemove);
    mainLayout->addWidget(btnImport);
    mainLayout->addStretch(1);

    setLayout(mainLayout);
}

void StandardTableEditor::init(StandardTableView *standardTableView)
{
    m_standardTableView = standardTableView;

    if (m_standardTableView->selectionModel()->selectedIndexes().isEmpty())
        btnRemove->setEnabled(false);

    connectActions();
    connect(m_standardTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &StandardTableEditor::onSelectionChanged);
}

void StandardTableEditor::setControlsEnabled(bool enabled)
{
    btnAdd->setEnabled(enabled);
    btnRemove->setEnabled(enabled);
    btnImport->setEnabled(enabled);
}

void StandardTableEditor::setImportEnabled(bool enabled)
{
    btnImport->setVisible(enabled);
    btnImport->setEnabled(enabled);
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

void StandardTableEditor::connectActions()
{
    connect(btnAdd,    &QPushButton::clicked, this, &StandardTableEditor::onAddItemClicked);
    connect(btnRemove, &QPushButton::clicked, this, &StandardTableEditor::onRemoveItemClicked);
    connect(btnImport, &QPushButton::clicked, this, &StandardTableEditor::onImportClicked);
}

void StandardTableEditor::disconnectActions()
{
    disconnect(btnAdd,    &QPushButton::clicked, this, &StandardTableEditor::onAddItemClicked);
    disconnect(btnRemove, &QPushButton::clicked, this, &StandardTableEditor::onRemoveItemClicked);
    disconnect(btnImport, &QPushButton::clicked, this, &StandardTableEditor::onImportClicked);
}

void StandardTableEditor::onAddItemClicked()
{
    if (!m_standardTableView)
        return;

    m_standardTableView->addRow();
}

void StandardTableEditor::onRemoveItemClicked()
{
    if (!m_standardTableView)
        return;

    m_standardTableView->removeSelectedRows();
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
    btnRemove->setEnabled(hasSelection);
}
