#include "Utilities.h"

#include <QApplication>
#include <QIcon>
#include <QPainter>
#include <QStylePainter>
#include <QRect>
#include <QResizeEvent>
#include <QStyleOptionFrame>
#include <QTextDocument>
#include <QTextLayout>

//-----------------------------------------------------------------------------
// DockWidget
//-----------------------------------------------------------------------------

DockWidgetProxyStyle::DockWidgetProxyStyle(QStyle *style)
    : QProxyStyle(style)
{}

int DockWidgetProxyStyle::pixelMetric(PixelMetric which,
    const QStyleOption *option, const QWidget *widget) const
{
    // Set the title margins and icon size.
    switch (which) {
    case QStyle::PM_DockWidgetTitleMargin:
        return 6;
    case QStyle::PM_SmallIconSize:
        return 24;
    default:
        return QProxyStyle::pixelMetric(which, option, widget);
    }
}

QIcon DockWidgetProxyStyle::standardIcon(QStyle::StandardPixmap icon,
    const QStyleOption *option, const QWidget *widget) const
{
    // Set custom float and close button icons.
    const QIcon floatIcon = QIcon(":/images/Restore_NoHalo_24x.png");
    const QIcon closeIcon = QIcon(":/images/Close_NoHalo_24x.png");

    switch (icon) {
    case QStyle::SP_TitleBarNormalButton:
        return floatIcon;
    case QStyle::SP_TitleBarCloseButton:
        return closeIcon;
    default:
        return QProxyStyle::standardIcon(icon, option, widget);
    }
}

void DockWidgetProxyStyle::drawControl(QStyle::ControlElement element,
    const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionDockWidget *dwOption;
    const QDockWidget *dw;

    if (element == CE_DockWidgetTitle &&
       (dwOption = qstyleoption_cast<const QStyleOptionDockWidget *>(option)) &&
       (dw = qobject_cast<const QDockWidget *>(widget)) &&
       !dw->titleBarWidget())
    {
        // Calculate the title bar height.
        int iconSize = dw->style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, dw);
        int buttonMargin = dw->style()->pixelMetric(QStyle::PM_DockWidgetTitleBarButtonMargin, nullptr, dw);
        int buttonHeight = iconSize + 2+buttonMargin;
        QFontMetrics titleFontMetrics = dw->fontMetrics();
        int titleMargin = dw->style()->pixelMetric(QStyle::PM_DockWidgetTitleMargin, nullptr, dw);
        int titleHeight = qMax(buttonHeight + 2, titleFontMetrics.height() + 2*titleMargin);

        // Draw a background rectangle for the title bar.
        QColor bgColor = dwOption->palette.window().color().darker(104);
        QRect bgRect = dwOption->rect;
        bgRect.setHeight(titleHeight);
        painter->fillRect(bgRect, bgColor);

        // Add some left padding to the title text.
        QStyleOptionDockWidget customOption = *dwOption;
        customOption.rect.setLeft(5);

        QProxyStyle::drawControl(element, &customOption, painter, widget);
        return;
    }

    QProxyStyle::drawControl(element, option, painter, widget);
}

DockWidget::DockWidget(const QString& title, QWidget *parent, Qt::WindowFlags flags)
{
    setWindowTitle(title);
    proxyStyle = new DockWidgetProxyStyle(style());
    setStyle(proxyStyle);
}

DockWidget::DockWidget(QWidget *parent, Qt::WindowFlags flags)
{
    proxyStyle = new DockWidgetProxyStyle(style());
    setStyle(proxyStyle);
}

//-----------------------------------------------------------------------------
// GridLayout
//-----------------------------------------------------------------------------

GridLayout::GridLayout() {
    setColumnMinimumWidth(0, 225);
    setColumnMinimumWidth(1, 175);
    setColumnStretch(0, 1);
    setColumnStretch(1, 2);
}

//-----------------------------------------------------------------------------
// BackgroundFrame
//-----------------------------------------------------------------------------

BackgroundFrame::BackgroundFrame() {
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setFrameShape(QFrame::StyledPanel);
}

//-----------------------------------------------------------------------------
// PlainTextEdit
//-----------------------------------------------------------------------------

PlainTextEdit::PlainTextEdit(QWidget *parent) : QPlainTextEdit(parent)
{
    QFont font;
    font.setFamily("Consolas");
    font.setStyleHint(QFont::TypeWriter);
    font.setFixedPitch(true);
    font.setPointSize(9);

    setFont(font);
    setWordWrapMode(QTextOption::NoWrap);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setReadOnly(true);
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

//-----------------------------------------------------------------------------
// DoubleLineEdit
//-----------------------------------------------------------------------------

DoubleLineEdit::DoubleLineEdit(QWidget *parent) : QLineEdit(parent)
{
    QDoubleValidator *validator = new QDoubleValidator(this);
    validator->setNotation(QDoubleValidator::StandardNotation);
    this->setValidator(validator);
}

DoubleLineEdit::DoubleLineEdit(double min, double max, int decimals, QWidget *parent)
    : QLineEdit(parent), m_min(min), m_max(max), m_decimals(decimals)
{
    QDoubleValidator *validator = new QDoubleValidator(min, max, decimals, this);
    validator->setNotation(QDoubleValidator::StandardNotation);
    this->setValidator(validator);
}

double DoubleLineEdit::value()
{
    bool ok = false;
    double v = this->text().toDouble(&ok);
    if (ok)
        return v;
    else
        return 0;
}

void DoubleLineEdit::setValue(double value)
{
    QString text = QString::number(value);
    this->setText(text);
}

//-----------------------------------------------------------------------------
// ReadOnlyLineEdit
//-----------------------------------------------------------------------------

ReadOnlyLineEdit::ReadOnlyLineEdit(QWidget *parent) : QLineEdit(parent)
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

void ReadOnlyLineEdit::paintEvent(QPaintEvent* event)
{
    QLineEdit::paintEvent(event);

    //QPainter painter(this);
    //QStyleOptionFrame option;
    //this->initStyleOption(&option);
    //option.state = QStyle::State_Raised;
    //this->style()->drawPrimitive(QStyle::PE_FrameLineEdit, &option, &painter, this);
}

//-----------------------------------------------------------------------------
// ReadOnlyTextEdit
//-----------------------------------------------------------------------------

ReadOnlyTextEdit::ReadOnlyTextEdit(QWidget *parent) : QTextEdit(parent)
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

void ReadOnlyTextEdit::setLineCount(const int rows)
{
    const QTextDocument *doc = this->document();
    const QFontMetrics fm(doc->defaultFont());
    const QMargins margins = this->contentsMargins();

    int height = fm.lineSpacing() * rows +
        (doc->documentMargin() + this->frameWidth()) * 2 +
        margins.top() + margins.bottom();

    this->setFixedHeight(height);
}

//-----------------------------------------------------------------------------
// StatusLabel
//-----------------------------------------------------------------------------

StatusLabel::StatusLabel(QWidget *parent) : QWidget(parent)
{
    m_iconLabel = new QLabel;
    m_textLabel = new QLabel;
    m_textLabel->setWordWrap(true);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_iconLabel, 0, Qt::AlignTop);
    mainLayout->addWidget(m_textLabel, 1, Qt::AlignTop);
    setLayout(mainLayout);
}

void StatusLabel::setSeverity(const int severity)
{
    // severity corresponds to enum severity_level in boost::log::trivial.

    static const QVector<QPixmap> pixmaps = {
        QPixmap(":/images/StatusAnnotations_Information_16xLG.png"),       // 0, trace
        QPixmap(":/images/StatusAnnotations_Information_16xLG.png"),       // 1, debug
        QPixmap(":/images/StatusAnnotations_Information_16xLG_color.png"), // 2, info
        QPixmap(":/images/StatusAnnotations_Warning_16xLG_color.png"),     // 3, warning
        QPixmap(":/images/StatusAnnotations_Invalid_16xLG_color.png"),     // 4, error
        QPixmap(":/images/StatusAnnotations_Critical_16xLG_color.png")     // 5, fatal
    };

    int index = 0;
    if (severity < 0)
        index = 0;
    else if (severity > 5)
        index = 5;
    else
        index = severity;

    m_iconLabel->setPixmap(pixmaps.at(index));
}

void StatusLabel::setText(const QString& text)
{
    m_textLabel->setText(text);
}
