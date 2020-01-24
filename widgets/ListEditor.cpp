// Copyright 2020 Dow, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "ListEditor.h"
#include "AppStyle.h"

#include <QBoxLayout>
#include <QComboBox>
#include <QDoubleValidator>
#include <QIcon>
#include <QKeyEvent>
#include <QListWidget>
#include <QListWidgetItem>
#include <QStyle>
#include <QToolButton>

ListEditor::ListEditor(QWidget *parent)
    : QWidget(parent)
{
    cboEditor = new QComboBox;
    cboEditor->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    btnAdd = new QToolButton;
    btnAdd->setIcon(this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_ActionAdd)));
    btnAdd->setToolButtonStyle(Qt::ToolButtonIconOnly);

    btnRemove = new QToolButton;
    btnRemove->setIcon(this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_ActionRemove)));
    btnRemove->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnRemove->setDisabled(true);

    lwEditor = new QListWidget;
    lwEditor->setSortingEnabled(true);
    lwEditor->setFlow(QListView::LeftToRight);
    lwEditor->setResizeMode(QListView::Adjust);
    lwEditor->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    lwEditor->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    lwEditor->setSpacing(2);

    int iconSize = this->style()->pixelMetric(QStyle::PM_SmallIconSize);
    int buttonMargin = this->style()->pixelMetric(QStyle::PM_ButtonMargin);
    lwEditor->setFixedHeight(iconSize + buttonMargin);

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

void ListEditor::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (cboEditor->hasFocus()) {
            emit onAddClicked();
            e->accept();
        }
        break;
    case Qt::Key_Delete:
        if (lwEditor->hasFocus()) {
            emit onRemoveClicked();
            e->accept();
        }
        break;
    default:
        break;
    }
}
