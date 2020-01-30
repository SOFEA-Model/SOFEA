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

#ifndef MONTECARLOLINEEDIT_H
#define MONTECARLOLINEEDIT_H

#include <QAction>
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QDoubleValidator>

#include "GenericDistribution.h"

class MonteCarloLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit MonteCarloLineEdit(QWidget *parent = nullptr);
    void setMinimum(double min);
    void setMaximum(double max);
    void setDecimals(int decimals);
    void setRange(double min, double max);
    GenericDistribution getDistribution() const;
    void setDistribution(const GenericDistribution &d);

private:
    QAction *m_selectDistributionAct;
    GenericDistribution m_distribution;
    QDoubleValidator *m_validator;
    bool m_distributionSet;
    QString m_distributionText;
    double m_previousValue;

    void init();
    void resetState();
    void clearDistribution();
    QString getDistributionText() const;

private slots:
    void onSelectDistribution();
    void onCursorPositionChanged(int oldPos, int newPos);
    void onTextEdited(const QString &text);

protected:
    void focusInEvent(QFocusEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // MONTECARLOLINEEDIT_H
