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

#ifndef MONTECARLODATETIMEEDIT_H
#define MONTECARLODATETIMEEDIT_H

#include <QAction>
#include <QDateTimeEdit>
#include <QEvent>
#include <QKeyEvent>

#include "core/DateTimeDistribution.h"

class MonteCarloDateTimeEdit : public QDateTimeEdit
{
    Q_OBJECT

public:
    explicit MonteCarloDateTimeEdit(QWidget *parent = nullptr);

    bool isModified() const;
    void setModified(bool);
    DateTimeDistribution getDistribution() const;
    void setDistribution(const DateTimeDistribution &d);

private:
    QAction *selectDistributionAct;
    bool modified = false;
    DateTimeDistribution currentDist;
    QDateTime previousValue;
    bool firstShowEvent = true;

    void reset();
    void clearDistribution();
    void resetToolButtonPosition();

private slots:
    void onSelectDistribution();
    void onCursorPositionChanged(int oldPos, int newPos);
    void onTextEdited(const QString &);
    void onDateTimeChanged(const QDateTime &dt);

protected:
    void focusInEvent(QFocusEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
};

#endif // MONTECARLODATETIMEEDIT_H
