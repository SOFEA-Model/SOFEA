#include "BackgroundFrame.h"

#include <QPaintEvent>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOptionTabWidgetFrame>
#include <QWidget>

BackgroundFrame::BackgroundFrame(QWidget *parent)
    : QFrame(parent)
{
    setAutoFillBackground(true);
}

void BackgroundFrame::paintEvent(QPaintEvent* event)
{
    QStylePainter p(this);
    QStyleOptionTabWidgetFrame opt;
    opt.rect = rect();
    p.drawPrimitive(QStyle::PE_FrameTabWidget, opt);
}
