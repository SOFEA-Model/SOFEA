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
    char format = (validator->notation() == QDoubleValidator::StandardNotation) ? 'f' : 'g';
    QString text = QString::number(value, format, validator->decimals());
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
