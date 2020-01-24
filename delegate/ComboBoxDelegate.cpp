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

#include "ComboBoxDelegate.h"

#include <QComboBox>

ComboBoxDelegate::ComboBoxDelegate(QAbstractItemModel *model, int column, QObject *parent)
    : QStyledItemDelegate(parent), m_editorModel(model), m_editorColumn(column)
{}

void ComboBoxDelegate::setEditorModel(QAbstractItemModel *model)
{
    m_editorModel = model;
}

void ComboBoxDelegate::setEditorModelColumn(int column)
{
    m_editorColumn = column;
}

QWidget* ComboBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    QComboBox *cb = new QComboBox(parent);
    cb->setFrame(false);
    cb->setModel(m_editorModel);
    cb->setModelColumn(m_editorColumn);

    connect(cb, QOverload<int>::of(&QComboBox::activated),
            this, &ComboBoxDelegate::emitCommitData);

    return cb;
}

void ComboBoxDelegate::setEditorData(QWidget *editor,
    const QModelIndex& index) const
{
    if (QComboBox *cb = qobject_cast<QComboBox *>(editor)) {
       int cbIndex = cb->findData(index.data(Qt::EditRole), Qt::EditRole);
       if (cbIndex >= 0)
           cb->setCurrentIndex(cbIndex);
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void ComboBoxDelegate::setModelData(QWidget *editor,
    QAbstractItemModel *model,
    const QModelIndex& index) const
{
    if (QComboBox *cb = qobject_cast<QComboBox *>(editor)) {
        model->setData(index, cb->currentData(Qt::EditRole), Qt::EditRole);
        // The current index is stored as Qt::UserRole.
        model->setData(index, cb->currentIndex(), Qt::UserRole);
    }
    else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}

void ComboBoxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem& option,
    const QModelIndex&) const
{
    editor->setGeometry(option.rect);
}

bool ComboBoxDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
    const QStyleOptionViewItem& option, const QModelIndex& index)
{
    emit editRequested();
    return false;
}

void ComboBoxDelegate::emitCommitData()
{
    // Notify other application components of the change.
    emit commitData(qobject_cast<QWidget *>(sender()));
}
