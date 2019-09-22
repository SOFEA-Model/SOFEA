#include "GridLayout.h"

#include <QWidget>

GridLayout::GridLayout(QWidget *parent)
    : QGridLayout(parent)
{
    setColumnMinimumWidth(0, 225);
    setColumnMinimumWidth(1, 175);
    setColumnStretch(0, 1);
    setColumnStretch(1, 2);
}
