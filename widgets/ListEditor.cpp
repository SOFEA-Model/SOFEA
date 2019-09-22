#include "ListEditor.h"

#include <QBoxLayout>
#include <QComboBox>
#include <QDoubleValidator>
#include <QIcon>
#include <QListWidget>
#include <QListWidgetItem>
#include <QToolButton>

ListEditor::ListEditor(QWidget *parent)
    : QWidget(parent)
{
    cboEditor = new QComboBox;
    cboEditor->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    btnAdd = new QToolButton;
    static const QIcon icoAdd = QIcon(":/images/Add_grey_16x.png");
    btnAdd->setIconSize(QSize(16, 16));
    btnAdd->setIcon(icoAdd);
    btnAdd->setToolButtonStyle(Qt::ToolButtonIconOnly);

    btnRemove = new QToolButton;
    static const QIcon icoRemove = QIcon(":/images/Remove_grey_16x.png");
    btnRemove->setIconSize(QSize(16, 16));
    btnRemove->setDisabled(true);
    btnRemove->setIcon(icoRemove);
    btnRemove->setToolButtonStyle(Qt::ToolButtonIconOnly);

    lwEditor = new QListWidget;
    lwEditor->setSortingEnabled(true);
    lwEditor->setFlow(QListView::LeftToRight);
    lwEditor->setResizeMode(QListView::Adjust);
    lwEditor->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    lwEditor->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    lwEditor->setSpacing(2);

    // Connections
    connect(cboEditor, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ListEditor::onCurrentIndexChanged);
    connect(btnAdd, &QToolButton::clicked, this, &ListEditor::onAddClicked);
    connect(btnRemove, &QToolButton::clicked, this, &ListEditor::onRemoveClicked);
    connect(lwEditor->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ListEditor::onSelectionChanged);

    // Layout
    QHBoxLayout *editorLayout = new QHBoxLayout;
    editorLayout->addWidget(lwEditor, 1);
    editorLayout->addWidget(btnAdd);
    editorLayout->addWidget(btnRemove);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(cboEditor);
    mainLayout->addLayout(editorLayout);
    mainLayout->addStretch(1);

    this->setLayout(mainLayout);
}


void ListEditor::addValue(double p)
{
    if ((p < minValue) || (p > maxValue))
        return;
    QListWidgetItem *item = new QListWidgetItem;
    item->setData(Qt::DisplayRole, p);
    lwEditor->addItem(item);
}

void ListEditor::setValues(std::vector<double> &values)
{
    lwEditor->clear();
    for (double p : values)
        addValue(p);
    resetLayout();
}

void ListEditor::clearValues()
{
    lwEditor->clear();
}

void ListEditor::setEditable(bool editable)
{
    cboEditor->setEditable(editable);
}

void ListEditor::setComboBoxItems(const QStringList &items)
{
    cboEditor->clear();
    cboEditor->addItems(items);
    cboEditor->setCurrentIndex(-1);
}

void ListEditor::setValidator(double min, double max, int decimals)
{
    QDoubleValidator *validator = new QDoubleValidator(min, max, decimals);
    cboEditor->setEditable(true);
    cboEditor->setValidator(validator);
    minValue = min;
    maxValue = max;
}

void ListEditor::setWhatsThis(const QString &text)
{
    cboEditor->setWhatsThis(text);
    btnAdd->setWhatsThis(text);
    btnRemove->setWhatsThis(text);
    lwEditor->setWhatsThis(text);
}

void ListEditor::resetLayout()
{
    lwEditor->setFixedHeight(lwEditor->sizeHintForRow(0) + 2 * lwEditor->frameWidth());
}

std::vector<double> ListEditor::values() const
{
    std::vector<double> values;

    for (int i = 0; i < lwEditor->count(); ++i) {
        QListWidgetItem *item = lwEditor->item(i);
        double p = item->data(Qt::DisplayRole).toDouble();
        values.push_back(p);
    }

    return values;
}

void ListEditor::onAddClicked()
{
    double p = cboEditor->currentText().toDouble();
    for (int i = 0; i < lwEditor->count(); ++i) {
        QListWidgetItem *current = lwEditor->item(i);
        if (current->data(Qt::DisplayRole).toDouble() == p)
            return;
    }

    addValue(p);
}

void ListEditor::onRemoveClicked()
{
    qDeleteAll(lwEditor->selectedItems());
}

void ListEditor::onCurrentIndexChanged(int)
{
    emit onAddClicked();
}

void ListEditor::onSelectionChanged(const QItemSelection&, const QItemSelection&)
{
    bool hasSelection = lwEditor->selectionModel()->hasSelection();
    btnRemove->setEnabled(hasSelection);
}
