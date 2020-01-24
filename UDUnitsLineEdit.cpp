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

    m_defaultPalette = this->palette();
    m_errorPalette = this->palette();
    m_errorPalette.setColor(QPalette::Text, Qt::red);

    connect(this, &UDUnitsLineEdit::textChanged,
            this, &UDUnitsLineEdit::onTextChanged);
}

void UDUnitsLineEdit::setConvertFrom(const QString &unit)
{
    m_convertFromText = unit;
}

void UDUnitsLineEdit::setBasePalette()
{
    QColor windowColor = QWidget::palette().window().color();
    m_defaultPalette.setColor(QPalette::Base, windowColor);
    m_errorPalette.setColor(QPalette::Base, windowColor);
    this->setPalette(m_defaultPalette);
}

QString UDUnitsLineEdit::parsedText()
{
    return m_parsedText;
}

double UDUnitsLineEdit::scaleFactor()
{
    // Return the scale factor if "from" and "to" units are set.
    if (!m_convertFromText.isEmpty() && !text().isEmpty()) {
        return m_scaleFactor;
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
        this->setPalette(m_defaultPalette);
        this->setToolTip(QString());
        m_parsedText = QString();
    }
    else {
        // Check for valid parse.
        UDUnits::Unit unit(text);

        if (unit.isValid()) {
            this->setPalette(m_defaultPalette);
            this->setToolTip(QString());
            m_parsedText = unit.toString();
        }
        else {
            this->setPalette(m_errorPalette);
            this->setToolTip(UDUnits::message);
            m_parsedText = QString();
        }
    }

    // Check for valid conversion.
    if (!m_convertFromText.isEmpty()) {
        UDUnits::Unit from(m_convertFromText);
        UDUnits::Unit to(text);

        if (UDUnits::Converter::isConvertible(from, to)) {
            this->setPalette(m_defaultPalette);
            this->setToolTip(QString());

            // Calculate the scale factor.
            UDUnits::Converter converter(from, to);
            m_scaleFactor = converter.convert(1.0);
        }
        else {
            this->setPalette(m_errorPalette);
            this->setToolTip(UDUnits::message);
        }
    }

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
