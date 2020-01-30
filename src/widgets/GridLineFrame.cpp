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

#include "GridLineFrame.h"

#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QWidget>

GridLineFrame::GridLineFrame(Qt::Orientation orientation, QWidget *parent)
    : QFrame(parent)
{
    setContentsMargins(0, 0, 0, 0);
    if (orientation == Qt::Horizontal)
        setFrameShape(QFrame::HLine);
    else
        setFrameShape(QFrame::VLine);
    setFrameShadow(QFrame::Plain);
    setLineWidth(1);
    setMidLineWidth(0);

    QStyleOptionViewItem opt;
    int hint = QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &opt);
    QColor color = static_cast<QRgb>(hint);

    QPalette palette = this->palette();
    palette.setColor(QPalette::WindowText, color);
    setPalette(palette);
}
