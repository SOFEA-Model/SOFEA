#include "DoubleLineEdit.h"

#include <QString>
#include <QWidget>

DoubleLineEdit::DoubleLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    validator = new QDoubleValidator(this);
    validator->setNotation(QDoubleValidator::StandardNotation);
    this->setValidator(validator);
}

DoubleLineEdit::DoubleLineEdit(double min, double max, int decimals, QWidget *parent)
    : QLineEdit(parent)
{
    validator = new QDoubleValidator(min, max, decimals, this);
    validator->setNotation(QDoubleValidator::StandardNotation);
    validator->setRange(min, max, decimals);
    this->setValidator(validator);
}

void DoubleLineEdit::setMinimum(double min)
{
    validator->setBottom(min);
}

void DoubleLineEdit::setMaximum(double max)
{
    validator->setTop(max);
}

void DoubleLineEdit::setDecimals(int prec)
{
    validator->setDecimals(prec);
}

void DoubleLineEdit::setValue(double value)
{
    QString text = QString::number(value);
    this->setText(text);
}

void DoubleLineEdit::setNotation(QDoubleValidator::Notation notation)
{
    validator->setNotation(notation);
}

double DoubleLineEdit::value() const
{
    bool ok = false;
    double v = this->text().toDouble(&ok);
    return ok ? v : 0;
}
