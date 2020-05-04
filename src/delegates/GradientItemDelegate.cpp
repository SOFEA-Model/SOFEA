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

#include "GradientItemDelegate.h"

#include <QApplication>
#include <QModelIndex>
#include <QPainter>
#include <QPixmap>
#include <QStyleOptionViewItem>

GradientItemDelegate::GradientItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

void GradientItemDelegate::paint(QPainter *painter,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.data(Qt::DecorationRole).canConvert<QLinearGradient>()) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    QLinearGradient gradient = qvariant_cast<QLinearGradient>(index.data(Qt::DecorationRole));

    // Draw background when selected.
    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());

    QRect rect = option.rect;
    int width = QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize) * 3;
    rect.setWidth(width);
    rect.adjust(3, 3, -3, -3);

    QPen pen(QApplication::palette().text().color());
    pen.setCosmetic(true);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(pen);
    painter->setBrush(gradient);
    painter->drawRect(rect);
    painter->restore();
}

QSize GradientItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex &) const
{
    int smallIconSize = qApp->style()->pixelMetric(QStyle::PM_SmallIconSize, &option);
    return QSize(smallIconSize * 3, smallIconSize);
}
