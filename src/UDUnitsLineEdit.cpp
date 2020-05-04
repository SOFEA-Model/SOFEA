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

#include "UDUnitsLineEdit.h"
#include "UDUnitsInterface.h"

#include <QDebug>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionFrame>

UDUnitsLineEdit::UDUnitsLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    setMaxLength(256);
    setAutoFillBackground(true);

    defaultPalette_ = QLineEdit::palette();
    normalPalette_ = QLineEdit::palette();
    errorPalette_ = QLineEdit::palette();
    errorPalette_.setColor(QPalette::Text, Qt::red);

    connect(this, &UDUnitsLineEdit::textChanged,
            this, &UDUnitsLineEdit::onTextChanged);
}

void UDUnitsLineEdit::setConvertFrom(const QString &unit)
{
    convertFromText_ = unit;
}

void UDUnitsLineEdit::setReadOnly(bool enable)
{
    QLineEdit::setReadOnly(enable);

    QColor textColor = enable ? defaultPalette_.color(QPalette::Disabled, QPalette::Text) :
                                defaultPalette_.color(QPalette::Normal, QPalette::Text);

    normalPalette_.setColor(QPalette::Text, textColor);

    updatePalette();
}

QString UDUnitsLineEdit::parsedText()
{
    return parsedText_;
}

double UDUnitsLineEdit::scaleFactor()
{
    // Return the scale factor if "from" and "to" units are set.
    if (!convertFromText_.isEmpty() && !text().isEmpty()) {
        return scaleFactor_;
    }
    else {
        return 1.0;
    }
}

void UDUnitsLineEdit::onTextChanged(const QString &text)
{
    // If both the "from" and "to" units are set, check conversion.
    // If only the "to" unit is set, check for valid parse.

    if (text.isEmpty()) {
        errorFlag_ = false;
        parsedText_ = QString();
        updatePalette();
        updateToolTip();
        emit unitsChanged();
        return;
    }

    // Check for valid parse.
    UDUnits::Unit unit(text);
    if (unit.isValid()) {
        errorFlag_ = false;
        parsedText_ = unit.toString();

        // Check for valid conversion.
        if (!convertFromText_.isEmpty()) {
            UDUnits::Unit from(convertFromText_);
            UDUnits::Unit to(text);
            if (UDUnits::Converter::isConvertible(from, to)) {
                UDUnits::Converter converter(from, to);
                scaleFactor_ = converter.convert(1.0);
            }
            else {
                errorFlag_ = true;
            }
        }
    }

    updatePalette();
    updateToolTip();
    emit unitsChanged();
}

void UDUnitsLineEdit::paintEvent(QPaintEvent* event)
{
    QLineEdit::paintEvent(event);

    //QPainter painter(this);
    //QStyleOptionFrame option;
    //this->initStyleOption(&option);
    //option.state = QStyle::State_Raised;
    //this->style()->drawPrimitive(QStyle::PE_FrameLineEdit, &option, &painter, this);
}

void UDUnitsLineEdit::updatePalette()
{
    setPalette(errorFlag_ ? errorPalette_ : normalPalette_);
}

void UDUnitsLineEdit::updateToolTip()
{
    setToolTip(errorFlag_ ? UDUnits::message : QString());
}
