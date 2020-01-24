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

#include "ColorIntervalDelegate.h"

#include <QApplication>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QMouseEvent>
#include <QStyle>

#include <QDebug>

// *** WORK IN PROGRESS ***
//
// TODO: ColorIntervalModel
// - Store current min, max range in UserRole for editor
// - Model calculates displayText

ColorIntervalDelegate::ColorIntervalDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

QWidget *ColorIntervalDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem&,
    const QModelIndex&) const
{
    QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
    //editor->setRange(m_min, m_max);
    //editor->setDecimals(m_decimals);
    //editor->setSingleStep(m_singleStep);
    editor->setFrame(false);

    return editor;
}

void ColorIntervalDelegate::setEditorData(QWidget *editor,
    const QModelIndex& index) const
{
    double value = index.data(Qt::EditRole).toDouble();
    QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox *>(editor);
    spinBox->setValue(value);
}

void ColorIntervalDelegate::setModelData(QWidget *editor,
    QAbstractItemModel *model,
    const QModelIndex& index) const
{
    QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox *>(editor);
    spinBox->interpretText();
    double value = spinBox->value();
    model->setData(index, value, Qt::EditRole);
}

void ColorIntervalDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem& option,
    const QModelIndex&) const
{
    editor->setGeometry(option.rect);
}

bool ColorIntervalDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
    const QStyleOptionViewItem& option, const QModelIndex& index)
{
    bool hasDecoration = option.features & QStyleOptionViewItem::HasDecoration;
    if (!hasDecoration)
        return false;

    if (event->type() == QEvent::MouseButtonRelease ||
        event->type() == QEvent::MouseButtonDblClick)
    {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        QPoint pos = me->pos() - option.rect.topLeft();

        const QStyle *s = QApplication::style();
        QRect iconRect = s->subElementRect(QStyle::SE_ItemViewItemDecoration, &option, option.widget);
        if (iconRect.contains(pos)) {
            qDebug() << "Decoration Hit";
            // Open QColorDialog
            return true;
        }
        else {
            qDebug() << "Other Hit";
            return false;
        }
    }

    return false;
}
