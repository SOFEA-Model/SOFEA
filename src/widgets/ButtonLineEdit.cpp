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

#include "ButtonLineEdit.h"

#include <QApplication>
#include <QFontMetrics>
#include <QMargins>
#include <QMenu>
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QProxyStyle>
#include <QStyle>
#include <QStyleOptionToolButton>
#include <QStylePainter>

#include <QDebug>

ButtonLineEdit::ButtonLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    updateButtonSize();
}

void ButtonLineEdit::setButtonText(const QString& text)
{
    buttonText_ = text;
    updateButtonSize();
}

QString ButtonLineEdit::buttonText()
{
    return buttonText_;
}

void ButtonLineEdit::setMinimumButtonSize(const QSize& minSize)
{
    minButtonSize_ = minSize;
    updateButtonSize();
}

QSize ButtonLineEdit::minimumButtonSize()
{
    return minButtonSize_;
}

void ButtonLineEdit::paintEvent(QPaintEvent *e)
{
    QLineEdit::paintEvent(e);

    QStyleOptionFrame frameOpt;
    initStyleOption(&frameOpt);
    QStyleOptionToolButton buttonOpt = buttonOption();

    int borderSize = 1;

    // Draw button clipped and overlay QLineEdit frame.
    QStylePainter painter(this);
    painter.save();
    painter.setClipRect(buttonOpt.rect.adjusted(borderSize, 0, 0, 0));
    buttonOpt.rect.adjust(-borderSize, 0, 0, 0);
    painter.drawPrimitive(QStyle::PE_PanelButtonTool, buttonOpt);
    painter.drawPrimitive(QStyle::PE_FrameLineEdit, frameOpt);
    buttonOpt.rect.adjust(borderSize, 0, 0, 0);
    painter.restore();

    // Draw separator.
    painter.save();
    QColor outline = this->palette().window().color().darker(140); // qfusionstyle_p_p.h
    painter.setPen(outline.lighter(110)); // qfusionstyle.cpp, CC_ComboBox
    const QPoint p1(buttonOpt.rect.left(), buttonOpt.rect.top() + borderSize);
    const QPoint p2(buttonOpt.rect.left(), buttonOpt.rect.bottom() - borderSize);
    painter.drawLine(p1, p2);
    painter.restore();

    // Draw button text.
    painter.drawControl(QStyle::CE_ToolButtonLabel, buttonOpt);
}

QStyleOptionToolButton ButtonLineEdit::buttonOption()
{
    QStyleOptionFrame frameOpt;
    initStyleOption(&frameOpt);

    QStyleOptionToolButton buttonOpt;
    buttonOpt.palette = this->palette();
    buttonOpt.features = QStyleOptionToolButton::None;
    buttonOpt.arrowType = Qt::NoArrow;
    buttonOpt.toolButtonStyle = Qt::ToolButtonTextOnly;
    buttonOpt.text = buttonText_;
    buttonOpt.state = frameOpt.state & (QStyle::State_Enabled | QStyle::State_MouseOver | QStyle::State_HasFocus);
    buttonOpt.state |= QStyle::State_KeyboardFocusChange;
    if (buttonPressed_)
        buttonOpt.state |= QStyle::State_Sunken;
    else
        buttonOpt.state &= ~QStyle::State_Sunken;

    int x = frameOpt.rect.left() + frameOpt.rect.width() - buttonSize_.width();
    int y = frameOpt.rect.top();
    int w = buttonSize_.width();
    int h = frameOpt.rect.height();
    buttonOpt.rect = QRect(x, y, w, h);

    return buttonOpt;
}

void ButtonLineEdit::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) {
        e->ignore();
        return;
    }

    if (hitButton(e->pos())) {
        if (e->button() != Qt::LeftButton) {
            e->ignore();
        }
        else {
            buttonPressed_ = true;
            update();
            emit buttonPressed();
            e->accept();
        }
    }
    else {
        QLineEdit::mousePressEvent(e);
    }
}

void ButtonLineEdit::mouseReleaseEvent(QMouseEvent *e)
{
    buttonPressed_ = false;
    update();

    if (e->button() != Qt::LeftButton) {
        e->ignore();
        return;
    }

    if (hitButton(e->pos())) {
        emit buttonClicked();
        e->accept();
    }
    else {
        e->ignore();
    }
}

bool ButtonLineEdit::hitButton(const QPoint& pos)
{
    QStyleOptionToolButton buttonOpt = buttonOption();
    QStyle::SubControl sc = style()->hitTestComplexControl(QStyle::CC_ToolButton, &buttonOpt, pos, this);
    return sc == QStyle::SC_ToolButton;
}

void ButtonLineEdit::updateButtonSize()
{
    QStyleOptionToolButton opt;
    opt.features = QStyleOptionToolButton::None;
    opt.arrowType = Qt::NoArrow;
    opt.toolButtonStyle = Qt::ToolButtonTextOnly;
    opt.text = buttonText_;

    QFontMetrics fm = opt.fontMetrics;
    QSize size = fm.boundingRect(buttonText_).size();

    int buttonMargin = style()->pixelMetric(QStyle::PM_ButtonMargin, &opt);
    int smallIconSize = style()->pixelMetric(QStyle::PM_SmallIconSize, &opt);

    size.rwidth() += buttonMargin * 2;
    size.rheight() += buttonMargin * 2;
    size = size.expandedTo(QSize(smallIconSize, smallIconSize));
    size = size.expandedTo(minButtonSize_);

    QMargins margins = textMargins();
    margins.setRight(size.width());
    setTextMargins(margins);

    buttonSize_ = size;
}
