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

#include "PointDelegate.h"

#include <QMetaType>
#include <QPainter>
#include <QPointF>

PointDelegate::PointDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

QString PointDelegate::displayText(const QVariant& value, const QLocale&) const
{
    if (value.type() == QMetaType::QPoint) {
        QPoint p = value.toPoint();
        return QString("(%1, %2)").arg(QString::number(p.x()))
                                  .arg(QString::number(p.y()));
    }
    else if (value.type() == QMetaType::QPointF) {
        QPointF p = value.toPointF();
        return QString("(%1, %2)").arg(QString::number(p.x()))
                                  .arg(QString::number(p.y()));
    }
    else {
        return QString();
    }
}

void PointDelegate::paint(QPainter *painter, const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());

    // Workaround for QPoint(0,0) which returns isNull() == true
    if ((index.data(Qt::DisplayRole).type() == QVariant::Point) ||
        (index.data(Qt::DisplayRole).type() == QVariant::PointF))
    {
        painter->drawText(option.rect,
                          displayText(index.data(), QLocale::system()),
                          option.displayAlignment);
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}
