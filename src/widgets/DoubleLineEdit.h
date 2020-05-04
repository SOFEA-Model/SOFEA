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

#pragma once

#include <QDoubleValidator>
#include <QLineEdit>
#include <QPalette>

class DoubleValidator;

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

class DoubleLineEdit : public QLineEdit
{
    Q_OBJECT

    Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged USER true)

public:
    explicit DoubleLineEdit(QWidget *parent = nullptr);
    DoubleLineEdit(double min, double max, int decimals, QWidget *parent = nullptr);
    
    void setMinimum(double min);
    void setMaximum(double max);
    void setRange(double min, double max);
    void setDecimals(int prec);
    void setNotation(QDoubleValidator::Notation notation);
    void setReadOnly(bool);
    void setValue(double value);
    double value() const;
    void setErrorPalette(bool);

signals:
    void valueChanged(double value);

private slots:
    void onTextChanged(const QString& text);
    void updatePalette();

private:
    DoubleValidator *validator_;
    QPalette defaultPalette_;
    QPalette errorPalette_;
};
