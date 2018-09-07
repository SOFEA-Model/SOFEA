#include "Utilities.h"

#include <QApplication>
#include <QPainter>
#include <QStylePainter>
#include <QRect>
#include <QResizeEvent>
#include <QStyleOptionFrame>
#include <QTextDocument>
#include <QTextLayout>

//-----------------------------------------------------------------------------
// GridLayout
//-----------------------------------------------------------------------------

GridLayout::GridLayout() {
    setColumnMinimumWidth(0, 175);
    setColumnMinimumWidth(1, 175);
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
    m_defaultPalette = this->palette();
    setAutoFillBackground(true);
    //setFrame(false);
    setReadOnly(true);
}

void ReadOnlyLineEdit::setBasePalette()
{
    QColor bgColor = QWidget::palette().window().color();
    m_defaultPalette.setColor(QPalette::Base, bgColor);
    this->setPalette(m_defaultPalette);
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
    m_defaultPalette = this->palette();
    setAutoFillBackground(true);
    setFrameShape(QFrame::NoFrame);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setWordWrapMode(QTextOption::NoWrap);
    setReadOnly(true);
}

void ReadOnlyTextEdit::setRowCount(const int rows)
{
    QTextDocument *doc = this->document();
    QFontMetrics fm(doc->defaultFont());
    QMargins margins = this->contentsMargins();

    int height = fm.lineSpacing() * rows +
        (doc->documentMargin() + this->frameWidth()) * 2 +
        margins.top() + margins.bottom();

    this->setFixedHeight(height);
}

void ReadOnlyTextEdit::paintEvent(QPaintEvent* event)
{
    // FIXME: drawing issues on scroll.

    QPainter painter(viewport());

    QStyleOptionFrame option;
    this->initStyleOption(&option);
    option.state = QStyle::State_Raised;
    this->style()->drawPrimitive(QStyle::PE_FrameLineEdit, &option, &painter, this);

    QTextEdit::paintEvent(event);
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
