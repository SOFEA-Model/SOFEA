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

#include "ProgressBarDelegate.h"

#include <QApplication>
#include <QModelIndex>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionProgressBar>

ProgressBarDelegate::ProgressBarDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

void ProgressBarDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    if (index.data().canConvert<int>())
    {
        QStyleOptionProgressBar progressBarOption;
        progressBarOption.state = QStyle::State_Enabled;
        progressBarOption.direction = QApplication::layoutDirection();
        progressBarOption.rect = option.rect;
        progressBarOption.fontMetrics = QApplication::fontMetrics();
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.textAlignment = Qt::AlignCenter;
        progressBarOption.textVisible = true;

        // Set the progress and text values of the style option.
        int progress = index.data(Qt::DisplayRole).toInt();
        progressBarOption.progress = progress < 0 ? 0 : progress;
        progressBarOption.text = QString::asprintf("%d%%", progressBarOption.progress);

        // Use base background fill even when selected.
        if (option.state & QStyle::State_Selected) {
            if (option.features & QStyleOptionViewItem::Alternate)
                painter->fillRect(option.rect, option.palette.alternateBase());
            else
                painter->fillRect(option.rect, option.palette.base());
        }

        // Draw a CE_ProgressBar without CE_ProgressBarGroove.
        QApplication::style()->drawControl(QStyle::CE_ProgressBarContents, &progressBarOption, painter);
        QApplication::style()->drawControl(QStyle::CE_ProgressBarLabel, &progressBarOption, painter);
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize ProgressBarDelegate::sizeHint(const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}
