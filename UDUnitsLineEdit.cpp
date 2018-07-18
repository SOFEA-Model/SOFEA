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
    setFrame(false);

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
    QColor bgColor = QWidget::palette().window().color();
    m_defaultPalette.setColor(QPalette::Base, bgColor);
    m_errorPalette.setColor(QPalette::Base, bgColor);
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

    QPainter painter(this);
    QStyleOptionFrame option;
    this->initStyleOption(&option);
    option.state = QStyle::State_Raised;
    this->style()->drawPrimitive(QStyle::PE_FrameLineEdit, &option, &painter, this);
}
