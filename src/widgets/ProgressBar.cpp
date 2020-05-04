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

#include "ProgressBar.h"

#include <QString>
#include <QProgressBar>
#ifdef Q_OS_WIN
#include <QWinTaskbarProgress>
#endif

ProgressBar::ProgressBar(QWidget *parent)
    : QProgressBar(parent), defaultPalette_(palette())
{
    QPalette p = this->palette();
    p.setColor(QPalette::Highlight, QColor(77, 230, 57)); // Green
    p.setColor(QPalette::HighlightedText, p.text().color());
    setPalette(p);

    connect(this, &QProgressBar::valueChanged, this, &ProgressBar::onValueChanged);
}

void ProgressBar::setText(const QString& text)
{
    setFormat(QString("%1%p%").arg(text));
}

void ProgressBar::setAutoHide(bool enable)
{
    autoHide_ = enable;
    setHidden(autoHide_ && state_ == NoProgress);
}

bool ProgressBar::autoHide()
{
    return autoHide_;
}

void ProgressBar::setState(ProgressState state)
{
    if (state == state_)
        return;

    state_ = state;

    setHidden(autoHide_ && state == NoProgress);

    QPalette p = this->palette();

    switch (state) {
    case NoProgress:
        p.setColor(QPalette::Highlight, QColor(77, 230, 57)); // Green
        p.setColor(QPalette::HighlightedText, p.text().color());
        break;
    case Indeterminate:
    case Running:
        p.setColor(QPalette::Highlight, QColor(77, 230, 57)); // Green
        p.setColor(QPalette::HighlightedText, p.text().color());
        break;
    case Error:
        p.setColor(QPalette::Highlight, QColor(255, 25, 25)); // Red
        p.setColor(QPalette::HighlightedText, defaultPalette_.highlightedText().color());
        break;
    case Paused:
        p.setColor(QPalette::Highlight, QColor(255, 240, 0)); // Yellow
        p.setColor(QPalette::HighlightedText, p.text().color());
        break;
    }

    setPalette(p);
    emit stateChanged(state);
}

ProgressBar::ProgressState ProgressBar::state()
{
    return state_;
}

#ifdef Q_OS_WIN
void ProgressBar::setTaskbarProgress(QWinTaskbarProgress *tp)
{
    if (!tp)
        return;

    connect(this, &QProgressBar::valueChanged, tp, &QWinTaskbarProgress::setValue);
    connect(this, &ProgressBar::stateChanged, [=](ProgressState state) {
         tp->setVisible(state != NoProgress);
         switch (state) {
         case NoProgress:
             break;
         case Indeterminate:
             tp->setRange(0, 0);
             break;
         case Running:
             tp->resume();
             break;
         case Error:
             tp->stop();
             break;
         case Paused:
             tp->pause();
             break;
         }
    });
}
#endif

void ProgressBar::onValueChanged(int value)
{
    if (value == 0 && minimum() == 0 && maximum() == 0)
        setState(Indeterminate);
    else if (value == minimum() || value == maximum())
        setState(NoProgress);
    else
        setState(Running);
}
