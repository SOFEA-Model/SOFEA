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

class DoubleValidator : public QDoubleValidator
{
public:
    DoubleValidator(QObject *parent = nullptr)
        : QDoubleValidator(parent)
    {}

    QValidator::State validate(QString &s, int &i) const
    {
        if (s.isEmpty() || s == locale().negativeSign())
            return QValidator::Intermediate;

        QChar decimalPoint = locale().decimalPoint();
        int charsAfterPoint = s.length() - s.indexOf(decimalPoint) -1;

        if (charsAfterPoint > decimals() && s.indexOf(decimalPoint) != -1)
            return QValidator::Invalid;

        bool ok;
        double value = locale().toDouble(s, &ok);

        if (ok && value >= bottom() && value <= top())
            return QValidator::Acceptable;
        else
            return QValidator::Invalid;
    }
};

DoubleLineEdit::DoubleLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    validator_ = new DoubleValidator(this);
    validator_->setNotation(QDoubleValidator::StandardNotation);
    setValidator(validator_);

    defaultPalette_ = QLineEdit::palette();
    errorPalette_ = QLineEdit::palette();
    errorPalette_.setColor(QPalette::Text, Qt::red);

    connect(this, &QLineEdit::textChanged, this, &DoubleLineEdit::onTextChanged);
    connect(validator_, &QValidator::changed, this, &DoubleLineEdit::updatePalette);
}

DoubleLineEdit::DoubleLineEdit(double min, double max, int decimals, QWidget *parent)
    : DoubleLineEdit(parent)
{
    validator_->setRange(min, max, decimals);
}

void DoubleLineEdit::setMinimum(double min)
{
    validator_->setBottom(min);
}

void DoubleLineEdit::setMaximum(double max)
{
    validator_->setTop(max);
}

void DoubleLineEdit::setRange(double min, double max)
{
    validator_->setBottom(min);
    validator_->setTop(max);
}

void DoubleLineEdit::setDecimals(int prec)
{
    validator_->setDecimals(prec);
}

void DoubleLineEdit::setNotation(QDoubleValidator::Notation notation)
{
    validator_->setNotation(notation);
}

void DoubleLineEdit::setReadOnly(bool enable)
{
    QLineEdit::setReadOnly(enable);

    QColor textColor = enable ? defaultPalette_.color(QPalette::Disabled, QPalette::Text) :
                                defaultPalette_.color(QPalette::Normal, QPalette::Text);

    QPalette p = this->palette();
    p.setColor(QPalette::Text, textColor);
    setPalette(p);
}

void DoubleLineEdit::setValue(double value)
{
    char format = (validator_->notation() == QDoubleValidator::StandardNotation) ? 'f' : 'g';
    QString text = QString::number(value, format, validator_->decimals());
    setText(text);
}

double DoubleLineEdit::value() const
{
    bool ok = false;
    double v = text().toDouble(&ok);
    return ok ? v : 0;
}

void DoubleLineEdit::setErrorPalette(bool enable)
{
    setPalette(enable ? errorPalette_ : defaultPalette_);
}

void DoubleLineEdit::onTextChanged(const QString& text)
{
    // editingFinished() and returnPressed() signals are only emitted
    // when the validator() returns QValidator::Acceptable.
    //
    // textEdited() and textChanged() signals are still emitted when
    // the validator returns QValidator::Intermediate.

    updatePalette();
    if (hasAcceptableInput())
        emit valueChanged(value());
}

void DoubleLineEdit::updatePalette()
{
    if (text().isEmpty() || text() == this->locale().negativeSign())
        setPalette(defaultPalette_);
    else if (hasAcceptableInput())
        setPalette(defaultPalette_);
    else
        setPalette(errorPalette_);
}
