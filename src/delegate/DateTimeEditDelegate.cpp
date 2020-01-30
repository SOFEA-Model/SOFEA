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

#include "DateTimeEditDelegate.h"

#include <QDateTimeEdit>

DateTimeEditDelegate::DateTimeEditDelegate(QDateTime min, QDateTime max, QObject *parent)
    : QStyledItemDelegate(parent), m_min(min), m_max(max)
{}

QWidget *DateTimeEditDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &,
    const QModelIndex &) const
{
    QDateTimeEdit *editor = new QDateTimeEdit(parent);
    editor->setDateTimeRange(m_min, m_max);
    editor->setTimeSpec(Qt::UTC);
    editor->setDisplayFormat("yyyy-MM-dd HH:mm");
    editor->setFrame(false);

    return editor;
}

void DateTimeEditDelegate::setEditorData(QWidget *editor,
    const QModelIndex &index) const
{
    QDateTime value = index.data(Qt::EditRole).toDateTime();
    value.setTimeSpec(Qt::UTC);
    QDateTimeEdit *dtEdit = static_cast<QDateTimeEdit *>(editor);
    dtEdit->setDateTime(value);
}

void DateTimeEditDelegate::setModelData(QWidget *editor,
    QAbstractItemModel *model,
    const QModelIndex &index) const
{
    QDateTimeEdit *dtEdit = static_cast<QDateTimeEdit *>(editor);
    dtEdit->interpretText();
    QDateTime value = dtEdit->dateTime();
    value.setTimeSpec(Qt::UTC);
    model->setData(index, value, Qt::EditRole);
}

void DateTimeEditDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option,
    const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}
