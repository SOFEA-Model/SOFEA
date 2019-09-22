#include "ReadOnlyLineEdit.h"

#include <QColor>
#include <QPalette>
#include <QWidget>

ReadOnlyLineEdit::ReadOnlyLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    setAutoFillBackground(true);
    setReadOnly(true);
}

void ReadOnlyLineEdit::setBasePalette()
{
    QPalette palette = this->palette();
    QColor windowColor = QWidget::palette().window().color();
    palette.setColor(QPalette::Base, windowColor);
    this->setPalette(palette);
}
