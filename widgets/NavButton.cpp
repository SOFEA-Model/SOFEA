#include "NavButton.h"

#include <QApplication>
#include <QFont>
#include <QPaintEvent>
#include <QPalette>
#include <QPen>
#include <QSize>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOptionButton>
#include <QWidget>

NavButton::NavButton(const QString& text, QWidget *parent)
    : QAbstractButton(parent)
{
    setText(text);
    setCheckable(true);
    setAutoExclusive(true);
}

void NavButton::paintEvent(QPaintEvent *)
{
    QStylePainter p(this);
    QStyleOptionButton opt;
    opt.initFrom(this);
    opt.palette = QApplication::palette();
    opt.text = this->text();

    // Use bold font for active buttons.
    if (this->isChecked()) {
        QFont font = this->font();
        font.setBold(true);
        p.setFont(font);
    }

    // Set the underline pen based on current state.
    QPen pen;
    pen.setWidth(6);

    if (this->isDown()) {
        pen.setBrush(opt.palette.shadow());
    }
    else if (this->isChecked()) {
        if (opt.state & QStyle::State_HasFocus) {
            pen.setBrush(QColor(52, 152, 219));
        }
        else {
            pen.setBrush(opt.palette.dark());
        }
    }
    else if (opt.state & QStyle::State_MouseOver) {
        pen.setBrush(opt.palette.mid());
    }
    else {
        pen.setBrush(opt.palette.midlight());
    }

    // Draw the background.
    p.setPen(Qt::NoPen);
    p.setBrush(opt.palette.window().color().lighter(102));
    p.drawRect(opt.rect);

    // Draw the label and underline.
    p.drawControl(QStyle::CE_PushButtonLabel, opt);
    p.setPen(pen);
    p.drawLine(opt.rect.bottomLeft(), opt.rect.bottomRight());
}

QSize NavButton::sizeHint() const
{
    return QSize(100, 60);
}
