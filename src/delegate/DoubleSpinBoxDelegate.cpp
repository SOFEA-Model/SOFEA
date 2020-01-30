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

#include "DoubleSpinBoxDelegate.h"

#include <QDoubleSpinBox>

DoubleSpinBoxDelegate::DoubleSpinBoxDelegate(double min, double max, int decimals, double singleStep, QObject *parent)
    : QStyledItemDelegate(parent), m_min(min), m_max(max), m_decimals(decimals), m_singleStep(singleStep)
{}

QWidget *DoubleSpinBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &,
    const QModelIndex &) const
{
    QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
    editor->setRange(m_min, m_max);
    editor->setDecimals(m_decimals);
    editor->setSingleStep(m_singleStep);
    editor->setFrame(false);

    return editor;
}

QString DoubleSpinBoxDelegate::displayText(const QVariant &value, const QLocale &) const
{
    return QString::number(value.toDouble(), 'f', m_decimals);
}

void DoubleSpinBoxDelegate::setEditorData(QWidget *editor,
    const QModelIndex &index) const
{
    double value = index.data(Qt::EditRole).toDouble();

    QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
    spinBox->setValue(value);
}

void DoubleSpinBoxDelegate::setModelData(QWidget *editor,
    QAbstractItemModel *model,
    const QModelIndex &index) const
{
    QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
    spinBox->interpretText();
    double value = spinBox->value();

    model->setData(index, value, Qt::EditRole);
}

void DoubleSpinBoxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option,
    const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}
