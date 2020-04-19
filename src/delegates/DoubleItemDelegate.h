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

#include <QStyledItemDelegate>

class DoubleItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit DoubleItemDelegate(QObject *parent = nullptr);
    
    DoubleItemDelegate(double min, double max, int decimals, bool fixed = false,
                       QObject *parent = nullptr);
    QWidget *createEditor(QWidget *parent,
        const QStyleOptionViewItem &, const QModelIndex &) const override;    
    QString displayText(const QVariant &, const QLocale &) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

private:
    const double m_min;
    const double m_max;
    const int m_decimals;
    const bool m_fixed;
};
