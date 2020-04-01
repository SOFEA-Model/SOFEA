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

#ifndef DATETIMEDISTRIBUTIONDIALOG_H
#define DATETIMEDISTRIBUTIONDIALOG_H

#include <QDialog>

#include "core/DateTimeDistribution.h"
#include "widgets/StandardTableView.h"
#include "widgets/StatusLabel.h"

QT_BEGIN_NAMESPACE
class QDateTimeEdit;
class QDialogButtonBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QStandardItemModel;
QT_END_NAMESPACE

class DateTimeDistributionDialog : public QDialog
{
    Q_OBJECT

public:
    DateTimeDistributionDialog(const DateTimeDistribution &d, QWidget *parent = nullptr);
    DateTimeDistribution getDistribution() const;

private slots:
    void onAddClicked();
    void onClearClicked();

private:
    void init();
    void resetTable();

public slots:
    void apply();
    void accept() override;
    void reject() override;

private:
    IntervalMap intervals;
    DateTimeDistribution currentDist;

    QDateTimeEdit *dteLower;
    QDateTimeEdit *dteUpper;
    QStandardItemModel *model;
    StandardTableView *view;
    QPushButton *btnAdd;
    QPushButton *btnClear;
    StatusLabel *lblStatus;
    QDialogButtonBox *buttonBox;
};

#endif // DATETIMEDISTRIBUTIONDIALOG_H
