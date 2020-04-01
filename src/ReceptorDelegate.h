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

#include <QApplication>
#include <QAbstractItemModel>
#include <QColorDialog>
#include <QFont>
#include <QLineEdit>
#include <QPainter>
#include <QPixmap>
#include <QRegExp>
#include <QRegExpValidator>
#include <QString>
#include <QStyledItemDelegate>

#include "core/Receptor.h"

//-----------------------------------------------------------------------------
// ReceptorGridDelegate
//-----------------------------------------------------------------------------

class ReceptorGridDelegate : public QStyledItemDelegate
{
public:
    explicit ReceptorGridDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {}

    QString ReceptorGridDelegate::displayText(const QVariant &value, const QLocale &) const override
    {
        if (value.canConvert<double>())
            return QString::number(value.toDouble(), 'f', 2);

        return value.toString();
    }

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override
    {
        QStyledItemDelegate::paint(painter, option, index);

        // Draw a vertical gridline on the left, except in the first column.
        // Horizontal gridlines are drawn in ReceptorTreeView::drawRow.
        if (index.column() == 0)
            return;

        QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &option));
        painter->save();
        painter->setPen(QPen(color));
        int x = (option.direction == Qt::LeftToRight) ? option.rect.left() : option.rect.right();
        painter->drawLine(x, option.rect.y(), x, option.rect.bottom());
        painter->restore();
    }
};

//-----------------------------------------------------------------------------
// ReceptorGroupDelegate
//-----------------------------------------------------------------------------

class ReceptorGroupDelegate : public ReceptorGridDelegate
{
public:
    explicit ReceptorGroupDelegate(QObject *parent = nullptr)
        : ReceptorGridDelegate(parent)
    {}

    QString ReceptorGroupDelegate::displayText(const QVariant &value, const QLocale &) const override
    {
        return value.toString();
    }

protected:
    QWidget *createEditor(QWidget *parent,
        const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        // Limit to alphanumeric characters, maximum length 8
        QLineEdit *editor = new QLineEdit(parent);
        editor->setFrame(false);
        editor->setMaxLength(8);
        QFont font = editor->font();
        font.setCapitalization(QFont::AllUppercase);
        editor->setFont(font);
        QRegExp rx("[A-Za-z0-9]+");
        QRegExpValidator *validator = new QRegExpValidator(rx, editor);
        editor->setValidator(validator);
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex& index) const override
    {
        QString value = index.data(Qt::EditRole).toString().toUpper();
        QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
        lineEdit->setText(value);
    }

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        editor->setGeometry(option.rect);
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt = option;

        // Disable focus rect for children.
        if (index.internalId() != 0)
            opt.state = opt.state & (~QStyle::State_HasFocus);

        QStyledItemDelegate::paint(painter, opt, index);
    }
};

//-----------------------------------------------------------------------------
// ReceptorColorDelegate
//-----------------------------------------------------------------------------

class ReceptorColorDelegate : public ReceptorGridDelegate
{
public:
    explicit ReceptorColorDelegate(QObject *parent = nullptr)
        : ReceptorGridDelegate(parent)
    {}

protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
        const QStyleOptionViewItem& option, const QModelIndex& index) override
    {
        if (event->type() != QEvent::MouseButtonDblClick)
            return false;

        // Editing only enabled for parent.
        if (index.internalId() != 0)
            return false;

        QColor color = index.data(Qt::DisplayRole).value<QColor>();
        QColorDialog dialog(color);
        dialog.setOptions(QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
        int rc = dialog.exec();
        if (rc == QDialog::Accepted) {
            color = dialog.currentColor();
            model->setData(index, color, Qt::EditRole);
        }

        return true;
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override
    {
        static const QPointF nodePoints[4] = {
            QPointF(0.1875, 0.375),
            QPointF(0.625, 0.1875),
            QPointF(0.8125, 0.625),
            QPointF(0.3125, 0.75)
        };

        static const QPointF ringPoints[8] = {
            QPointF(0.5, 0.15625),
            QPointF(0.75, 0.25),
            QPointF(0.84375, 0.5),
            QPointF(0.75, 0.75),
            QPointF(0.5, 0.84375),
            QPointF(0.25, 0.75),
            QPointF(0.15625, 0.5),
            QPointF(0.25, 0.25)
        };

        static const QPointF gridPoints[9] = {
            QPointF(0.1875, 0.1875),
            QPointF(0.5, 0.1875),
            QPointF(0.8125, 0.1875),
            QPointF(0.1875, 0.5),
            QPointF(0.5, 0.5),
            QPointF(0.8125, 0.5),
            QPointF(0.1875, 0.8125),
            QPointF(0.5, 0.8125),
            QPointF(0.8125, 0.8125)
        };

        // Skip children and disable focus rect.
        if (index.internalId() != 0) {
            QStyleOptionViewItem opt = option;
            opt.state = option.state & (~QStyle::State_HasFocus);
            QStyledItemDelegate::paint(painter, opt, index);
            return;
        }

        // Draw background when selected.
        if (option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());
        else
            painter->fillRect(option.rect, option.palette.window());

        QVariant groupTypeVar = index.data(Qt::UserRole);
        QVariant colorVar = index.data(Qt::DisplayRole);
        if (!groupTypeVar.canConvert(QVariant::Int) || !colorVar.canConvert(QVariant::Color))
            return;

        ReceptorGroupType groupType = static_cast<ReceptorGroupType>(groupTypeVar.value<int>());
        QBrush brush = colorVar.value<QColor>();

        int iconSize = QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize);
        const int x = option.rect.center().x() - iconSize / 2;
        const int y = option.rect.center().y() - iconSize / 2;
        const double radius = 1.25 / 16.;

        painter->save();
        painter->translate(x, y);
        painter->scale(iconSize, iconSize);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(brush);

        switch (groupType) {
        case ReceptorGroupType::Node:
            for (int i = 0; i < sizeof nodePoints / sizeof nodePoints[0]; ++i)
                painter->drawEllipse(nodePoints[i], radius, radius);
            break;
        case ReceptorGroupType::Ring:
            for (int i = 0; i < sizeof ringPoints / sizeof ringPoints[0]; ++i)
                painter->drawEllipse(ringPoints[i], radius, radius);
            break;
        case ReceptorGroupType::Grid:
            for (int i = 0; i < sizeof gridPoints / sizeof gridPoints[0]; ++i)
                painter->drawEllipse(gridPoints[i], radius, radius);
            break;
        default:
            break;
        }

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option,
        const QModelIndex &index) const override
    {
        const int iconSize = QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize);
        return QSize(iconSize * 1.5, iconSize);
    }
};
