#pragma once

#include <QWidget>
#include <QVector>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleValidator>
#include <QFont>
#include <QIcon>
#include <QLabel>
#include <QListWidget>
#include <QSettings>
#include <QToolButton>
#include <QToolBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <QDebug>

class ListEditor : public QWidget
{
private:
    QComboBox *cboEditor;
    QListWidget *lwEditor;
    QToolButton *btnAdd;
    QToolButton *btnRemove;
    double minValue;
    double maxValue;

public:
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
        cboEditor->setValidator(validator);
        minValue = min;
        maxValue = max;
    }

    void ListEditor::resetLayout()
    {
        lwEditor->setFixedHeight(lwEditor->sizeHintForRow(0) + 2 * lwEditor->frameWidth());
        cboEditor->setFixedHeight(lwEditor->height());
        cboEditor->setStyleSheet("font-size: 8pt;");
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

public slots:
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

public:
    ListEditor(QWidget *parent = nullptr) : QWidget(parent)
    {
        cboEditor = new QComboBox;
        cboEditor->setEditable(true);
        cboEditor->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

        btnAdd = new QToolButton;
        const QIcon icoAdd = QIcon(":/images/Add_grey_16x.png");
        btnAdd->setIcon(icoAdd);
        btnAdd->setToolButtonStyle(Qt::ToolButtonIconOnly);

        btnRemove = new QToolButton;
        const QIcon icoRemove = QIcon(":/images/Remove_grey_16x.png");
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

        setLayout(mainLayout);
    }
};
