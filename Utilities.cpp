#include "Utilities.h"

#include <QApplication>
#include <QPainter>
#include <QStyleOptionFrame>

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
    currentPalette = this->palette();
    setFrame(false);
    setReadOnly(true);
}

void ReadOnlyLineEdit::paintEvent(QPaintEvent* event)
{
    // Disable the frame in constructor, call base paintEvent, then redraw custom frame.

    QLineEdit::paintEvent(event);

    QPainter painter(this);
    QStyleOptionFrame option;
    this->initStyleOption(&option);
    option.state = QStyle::State_Raised;
    this->style()->drawPrimitive(QStyle::PE_FrameLineEdit, &option, &painter, this);
}
