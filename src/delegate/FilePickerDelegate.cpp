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

#include "FilePickerDelegate.h"

#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontMetrics>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>

FilePickerDelegate::FilePickerDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

QString FilePickerDelegate::dialogCaption() const {
    return dialogCaption_;
}

QString FilePickerDelegate::dialogDirectory() const {
    return dialogDirectory_;
}

QString FilePickerDelegate::dialogFilter() const {
    return dialogFilter_;
}

QFileDialog::FileMode FilePickerDelegate::dialogFileMode() const {
    return dialogFileMode_;
}

void FilePickerDelegate::setDialogCaption(const QString& caption) {
    dialogCaption_ = caption;
}

void FilePickerDelegate::setDialogDirectory(const QString& dir) {
    dialogDirectory_ = dir;
}

void FilePickerDelegate::setDialogFilter(const QString& filter) {
    dialogFilter_ = filter;
}

void FilePickerDelegate::setDialogFileMode(QFileDialog::FileMode mode) {
    dialogFileMode_ = mode;
}

void FilePickerDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int smallIconSize = qApp->style()->pixelMetric(QStyle::PM_SmallIconSize, &option, nullptr);
    int frameWidth = qApp->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &option, nullptr);

    QStyleOptionButton buttonOption;
    buttonOption.direction = qApp->layoutDirection();
    buttonOption.palette = qApp->palette();
    buttonOption.state = QStyle::State_Enabled | QStyle::State_Raised;
    buttonOption.iconSize = QSize(smallIconSize, smallIconSize);
    buttonOption.features = QStyleOptionButton::Flat;
    buttonOption.text = "...";

    //QStyleOptionToolButton buttonOption;
    //buttonOption.direction = qApp->layoutDirection();
    //buttonOption.palette = qApp->palette();
    //buttonOption.state = QStyle::State_Enabled | QStyle::State_Raised;
    //buttonOption.arrowType = Qt::NoArrow;
    //buttonOption.iconSize = QSize(smallIconSize, smallIconSize);
    //buttonOption.features = QStyleOptionToolButton::None;
    //buttonOption.toolButtonStyle = Qt::ToolButtonTextOnly;
    //buttonOption.subControls = QStyle::SC_ToolButton;
    //buttonOption.font = option.font;
    //buttonOption.text = "...";

    QFontMetrics fm = option.fontMetrics;
    int buttonTextWidth = fm.boundingRect(QString(" ... ")).width();
    int buttonWidth = qMax(smallIconSize, buttonTextWidth) + 2 * frameWidth;

    int x = option.rect.left() + option.rect.width() - buttonWidth;
    int y = option.rect.top();
    int w = buttonWidth;
    int h = option.rect.height();
    buttonOption.rect = QRect(x, y, w, h);

    QStyleOptionViewItem itemOption = option;
    itemOption.rect.adjust(0, 0, -buttonWidth, 0);

    qApp->style()->drawControl(QStyle::CE_PushButtonBevel, &buttonOption, painter);
    qApp->style()->drawControl(QStyle::CE_PushButtonLabel, &buttonOption, painter);

    QStyledItemDelegate::paint(painter, itemOption, index);
}

bool FilePickerDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent *e = static_cast<QMouseEvent *>(event);
        int xpos = e->x();
        int ypos = e->y();

        int smallIconSize = qApp->style()->pixelMetric(QStyle::PM_SmallIconSize, &option, nullptr);
        int frameWidth = qApp->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &option, nullptr);
        QFontMetrics fm = option.fontMetrics;
        int buttonTextWidth = fm.boundingRect(QString(" ... ")).width();
        int buttonWidth = qMax(smallIconSize, buttonTextWidth) + 2 * frameWidth;

        int x = option.rect.left() + option.rect.width() - buttonWidth;
        int y = option.rect.top();
        int w = buttonWidth;
        int h = option.rect.height();

        if (xpos > x && xpos < x + w && ypos > y && ypos < y + h)
        {
            QString currentPath = model->data(index, Qt::DisplayRole).value<QString>();
            QFileInfo fi(currentPath);
            QString dir = dialogDirectory_;
            if (!currentPath.isEmpty() && fi.isRelative()) {
                QDir currentDir(dialogDirectory_ + QDir::separator() + fi.path());
                if (currentDir.exists())
                    dir = currentDir.canonicalPath();
            }

            QFileDialog dialog(nullptr, dialogCaption_, dir, dialogFilter_);
            dialog.setFileMode(dialogFileMode_);
            if (dialog.exec())
                model->setData(index, dialog.selectedFiles().front(), Qt::EditRole);

            return true;
        }
    }

    return false;
}
