#include "CollapsibleGroupBox.h"

#include <QChildEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QStyle>
#include <QStyleOptionGroupBox>
#include <QStylePainter>

static QStyleOptionGroupBox CollapsibleGroupBoxStyleOption(const CollapsibleGroupBox *p)
{
    QStyleOptionGroupBox option;
    option.init(p);
    option.text = p->title();
    option.lineWidth = 1;
    option.midLineWidth = 0;
    option.textAlignment = Qt::AlignLeft;
    option.subControls = QStyle::SC_None;
    if (!p->title().isEmpty())
        option.subControls |= QStyle::SC_GroupBoxLabel;

    return option;
}

CollapsibleGroupBox::CollapsibleGroupBox(QWidget *parent)
    : QGroupBox(parent), collapsed_(false), pressed_(false)
{}

CollapsibleGroupBox::CollapsibleGroupBox(const QString& title, QWidget *parent)
    : QGroupBox(parent), collapsed_(false), pressed_(false)
{
    setTitle(title);
}

QRect CollapsibleGroupBox::textRect()
{
    QStyleOptionGroupBox option = CollapsibleGroupBoxStyleOption(this);
    option.subControls |= QStyle::SC_GroupBoxCheckBox;
    return style()->subControlRect(QStyle::CC_GroupBox, &option, QStyle::SC_GroupBoxLabel, this);
}

QRect CollapsibleGroupBox::collapsedRect()
{
    QStyleOptionGroupBox option = CollapsibleGroupBoxStyleOption(this);
    option.subControls |= QStyle::SC_GroupBoxCheckBox;
    return style()->subControlRect(QStyle::CC_GroupBox, &option, QStyle::SC_GroupBoxCheckBox, this);
}

void CollapsibleGroupBox::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QStyleOptionGroupBox option = CollapsibleGroupBoxStyleOption(this);

    QRect tRect = textRect();
    QRect cRect = collapsedRect();

    // Draw a custom frame.
    const int x1 = option.rect.left();
    const int y1 = option.rect.top();
    const int x2 = option.rect.right();
    const int y2 = tRect.bottom();
    QRect headerRect = QRect(QPoint{x1, y1}, QPoint{x2, y2});

    painter.save();

    painter.fillRect(headerRect, option.palette.button());

    QStyleOptionFrame fropt;
    fropt.QStyleOption::operator=(option);
    fropt.features = QStyleOptionFrame::Rounded;
    fropt.lineWidth = option.lineWidth;
    fropt.midLineWidth = option.midLineWidth;
    fropt.rect = headerRect;

    style()->drawPrimitive(QStyle::PE_Frame, &fropt, &painter, this);

    // Adjust the rects for the frame.
    cRect.moveLeft(cRect.left() + 5);
    tRect.moveLeft(tRect.left() + 5);

    // Draw text.
    if ((option.subControls & QStyle::SC_GroupBoxLabel) && !option.text.isEmpty()) {
         painter.setPen(QPen(option.palette.windowText(), 1));
         style()->drawItemText(&painter, tRect,  Qt::TextShowMnemonic | Qt::AlignLeft,
                               option.palette, option.state & QStyle::State_Enabled, option.text, QPalette::NoRole);
    }

    painter.restore();

    // Draw indicator.
    QStyleOption indicatorOption;
    indicatorOption.rect = collapsedRect();
    indicatorOption.state = QStyle::State_Children;
    if (!collapsed())
        indicatorOption.state |= QStyle::State_Open;
  
    style()->drawPrimitive(QStyle::PE_IndicatorBranch, &indicatorOption, &painter);
}

QSize CollapsibleGroupBox::minimumSizeHint() const
{
    QStyleOptionGroupBox option = CollapsibleGroupBoxStyleOption(this);

    int baseWidth = fontMetrics().horizontalAdvance(this->title() + QLatin1Char(' '));
    int baseHeight = fontMetrics().lineSpacing();

    baseWidth += style()->pixelMetric(QStyle::PM_IndicatorWidth);
    baseHeight = qMax(baseHeight, style()->pixelMetric(QStyle::PM_IndicatorHeight)) + 3;

    QSize baseSize(baseWidth, baseHeight);

    if (collapsed_)
        return baseSize;

    baseSize = QWidget::minimumSizeHint().expandedTo(baseSize);
    return style()->sizeFromContents(QStyle::CT_GroupBox, &option, baseSize, this);
}

void CollapsibleGroupBox::mousePressEvent(QMouseEvent* e)
{
    QRect cRect = collapsedRect();
    pressed_ = cRect.contains(e->pos());
}

void CollapsibleGroupBox::mouseMoveEvent(QMouseEvent* e)
{
    if (pressed_) {
        QRect cRect = collapsedRect();
        pressed_ = cRect.contains(e->pos());
    }
}

void CollapsibleGroupBox::mouseReleaseEvent(QMouseEvent* e)
{
    if (pressed_) {
        QRect cRect = collapsedRect();
        pressed_ = cRect.contains(e->pos());
    }

    if (pressed_)
        setCollapsed(!collapsed());
}

void CollapsibleGroupBox::childEvent(QChildEvent* c)
{
    if ((c->type() == QEvent::ChildAdded) && c->child()->isWidgetType()) {
        QWidget* w = static_cast<QWidget *>(c->child());
        if (!collapsed_ && !w->testAttribute(Qt::WA_ForceDisabled))
            w->setEnabled(true);
        else if (w->isEnabled()) {
            w->setEnabled(false);
            w->setAttribute(Qt::WA_ForceDisabled, false);
        }
    }
    QGroupBox::childEvent(c);
}

bool CollapsibleGroupBox::collapsed() const
{
    return this->collapsed_;
}

void CollapsibleGroupBox::setCollapsed(bool v)
{
    if (v == collapsed_)
        return;

    collapsed_ = v;
    QSize minSize = this->minimumSizeHint();
    
    if (collapsed_) {
        setChildrenEnabled(false);
        setMinimumHeight(minSize.height());
        setMaximumHeight(minSize.height());
    }
    else {
        setChildrenEnabled(true);
        setMinimumHeight(minSize.height());
        setMaximumHeight(QWIDGETSIZE_MAX);
    }
    
    updateGeometry();
    update();
}

void CollapsibleGroupBox::setChildrenEnabled(bool enabled)
{
    QObjectList childList = this->children();
    for (int i = 0; i < childList.size(); ++i) {
        QObject* o = childList.at(i);
        if (o->isWidgetType()) {
            QWidget* w = static_cast<QWidget*>(o);
            if (enabled && !w->testAttribute(Qt::WA_ForceDisabled))
                w->setEnabled(true);
            else if (w->isEnabled()) {
                w->setEnabled(false);
                w->setAttribute(Qt::WA_ForceDisabled, false);
            }
        }
    }
}
