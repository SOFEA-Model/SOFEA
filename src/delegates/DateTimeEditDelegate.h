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

#pragma once

#include <QDateTime>
#include <QStyledItemDelegate>

class DateTimeEditDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    DateTimeEditDelegate(QDateTime min = QDateTime(QDate(1800, 1, 1), QTime(0, 0, 0, 0)),
                         QDateTime max = QDateTime(QDate(2100, 12, 31), QTime(23, 59, 59, 999)),
                         QObject *parent = nullptr);
    
    QWidget *createEditor(QWidget *parent,
        const QStyleOptionViewItem &, const QModelIndex &) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor,
        QAbstractItemModel *model, const QModelIndex &) const override;
    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    const QDateTime m_min;
    const QDateTime m_max;
};
