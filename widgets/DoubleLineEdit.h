#pragma once

#include <QLineEdit>
#include <QDoubleValidator>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

class DoubleLineEdit : public QLineEdit
{
public:
    explicit DoubleLineEdit(QWidget *parent = nullptr);
    DoubleLineEdit(double min, double max, int decimals, QWidget *parent = nullptr);
    
    void setMinimum(double min);
    void setMaximum(double max);
    void setDecimals(int prec);
    void setNotation(QDoubleValidator::Notation notation);
    void setValue(double value);
    double value() const;
    
private:
    QDoubleValidator *validator;
};
