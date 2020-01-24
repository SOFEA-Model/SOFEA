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

#ifndef UDUNITSLINEEDIT_H
#define UDUNITSLINEEDIT_H

#include <QLineEdit>
#include <QPaintEvent>
#include <QPalette>

class UDUnitsLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit UDUnitsLineEdit(QWidget *parent = nullptr);
    void setBasePalette();
    void setConvertFrom(const QString &unit);
    QString parsedText();
    double scaleFactor();

signals:
    void unitsChanged();

private slots:
    void onTextChanged(const QString &text);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPalette m_defaultPalette;
    QPalette m_errorPalette;
    QString m_parsedText;
    QString m_convertFromText;
    double m_scaleFactor;
};

#endif // UDUNITSLINEEDIT_H
