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

#include "MenuLineEdit.h"

#include <QMenu>
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QStyle>
#include <QStyleOptionComboBox>

#include <QDebug>

MenuLineEdit::MenuLineEdit(QWidget *parent)
    : QLineEdit(parent)
{}

void MenuLineEdit::setMenu(QMenu *menu)
{
    menu_ = menu;
    updateTextMargins();
}

QMenu * MenuLineEdit::menu()
{
    return menu_;
}

void MenuLineEdit::showMenu()
{
    if (!menu_)
        return;

    int x = width() - menu_->sizeHint().width();
    int y = height();
    QPoint pos = QPoint(x, y);
    QPoint globalPos = mapToGlobal(pos);

    menu_->exec(globalPos);
}

void MenuLineEdit::paintEvent(QPaintEvent *e)
{
    // Draw the QStyleOptionFrame and editor.
    QLineEdit::paintEvent(e);
    if (!menu_)
        return;

    // Overlay QStyle::SC_ComboBoxArrow.
    QStyleOptionComboBox opt;
    opt.initFrom(this); // state, direction, rect, palette, fontMetrics, styleObject
    opt.editable = true;
    opt.frame = false;
    opt.subControls = QStyle::SC_ComboBoxArrow;
    if (arrowState_ == QStyle::State_Sunken)
        opt.state |= QStyle::State_Sunken;
    else
        opt.state &= ~QStyle::State_Sunken;
    if (this->isReadOnly())
        opt.state &= ~QStyle::State_Enabled;

    QPainter p(this);

    style()->drawComplexControl(QStyle::CC_ComboBox, &opt, &p, this);
}

void MenuLineEdit::mousePressEvent(QMouseEvent *e)
{
    if (!menu_) {
        QLineEdit::mousePressEvent(e);
        return;
    }

    QStyleOptionComboBox opt;
    opt.initFrom(this);
    opt.editable = true;
    opt.subControls = QStyle::SC_All;

    QStyle::SubControl hoverControl = style()->hitTestComplexControl(QStyle::CC_ComboBox, &opt, e->pos(), this);

    if (hoverControl == QStyle::SC_ComboBoxArrow) {
        e->accept();
        if (this->isReadOnly())
            return;
        arrowState_ = QStyle::State_Sunken;
        update();
        showMenu();
        arrowState_ = QStyle::State_None;
        update();
    }
    else {
        QLineEdit::mousePressEvent(e);
    }
}

void MenuLineEdit::updateTextMargins()
{
    if (!menu_)
        return;

    QStyleOptionComboBox opt;
    opt.initFrom(this);
    opt.editable = true;
    opt.subControls = QStyle::SC_All;

    QRect arrowRect = style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxArrow, this);

    QMargins margins = this->textMargins();
    margins.setRight(arrowRect.width());
    setTextMargins(margins);
}
