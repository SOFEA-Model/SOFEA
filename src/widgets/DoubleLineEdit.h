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
    void setRange(double min, double max);
    void setDecimals(int prec);
    void setNotation(QDoubleValidator::Notation notation);
    void setValue(double value);
    double value() const;
    
private:
    QDoubleValidator *validator;
};
