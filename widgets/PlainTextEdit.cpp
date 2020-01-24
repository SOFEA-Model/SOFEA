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

#include "PlainTextEdit.h"

#include <QApplication>
#include <QFont>
#include <QFontMetrics>
#include <QMargins>
#include <QString>
#include <QStylePainter>
#include <QTextDocument>
#include <QTextOption>
#include <QWidget>

PlainTextEdit::PlainTextEdit(QWidget *parent)
    : QPlainTextEdit(parent)
{
    QFont font;
    font.setFamily("Consolas");
    font.setStyleHint(QFont::TypeWriter);
    font.setFixedPitch(true);
    font.setPointSize(QApplication::font().pointSize());
    setFont(font);
}

void PlainTextEdit::setLineCount(int lines)
{
    const QTextDocument *doc = this->document();
    const QFontMetrics fm = this->fontMetrics();
    const QMargins margins = this->contentsMargins();

    int height = fm.lineSpacing() * lines +
        (doc->documentMargin() + this->frameWidth()) * 2 +
        margins.top() + margins.bottom();

    this->setFixedHeight(height);
}

void PlainTextEdit::appendPlainText(const std::string &text)
{
    QString qstr = QString::fromStdString(text);
    QPlainTextEdit::appendPlainText(qstr);
}

void PlainTextEdit::insertPlainText(const std::string &text)
{
    QString qstr = QString::fromStdString(text);
    QPlainTextEdit::insertPlainText(qstr);
}

void PlainTextEdit::setPlainText(const std::string &text)
{
    QString qstr = QString::fromStdString(text);
    QPlainTextEdit::setPlainText(qstr);
}
