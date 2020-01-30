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

#include "ReadOnlyTextEdit.h"

#include <QColor>
#include <QFontMetrics>
#include <QMargins>
#include <QPalette>
#include <QTextDocument>
#include <QWidget>

ReadOnlyTextEdit::ReadOnlyTextEdit(QWidget *parent)
    : QTextEdit(parent)
{
    setReadOnly(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAutoFillBackground(true);

    QPalette palette = this->palette();
    QColor baseColor = QWidget::palette().window().color().lighter(104);
    palette.setColor(QPalette::Base, baseColor);
    this->setPalette(palette);
}

void ReadOnlyTextEdit::setLineCount(int lines)
{
    const QTextDocument *doc = this->document();
    const QFontMetrics fm(doc->defaultFont());
    const QMargins margins = this->contentsMargins();

    int height = fm.lineSpacing() * lines +
        (doc->documentMargin() + this->frameWidth()) * 2 +
        margins.top() + margins.bottom();

    this->setFixedHeight(height);
}
