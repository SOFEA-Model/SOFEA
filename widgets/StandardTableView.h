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

#include <QTableView>
#include <QDateTime>
#include <QKeyEvent>
#include <QVariant>

#include <vector>

class StandardTableView : public QTableView
{
    Q_OBJECT
public:
    explicit StandardTableView(QWidget *parent = nullptr);
    void setAutoFilterEnabled(bool enabled);
    void addRow();
    void selectLastRow();
    void removeSelectedRows();
    bool moveSelectedRows(int offset);
    void copyClipboard();
    void setDoubleLineEditForColumn(int column, double min, double max, int decimals, bool fixed = false);
    void setSpinBoxForColumn(int column, int min, int max, int singleStep);
    void setDoubleSpinBoxForColumn(int column, double min, double max, int decimals, double singleStep);
    void setDateTimeEditForColumn(int column,
                                  QDateTime min = QDateTime(QDate(1800, 1, 1), QTime(0, 0, 0, 0)),
                                  QDateTime max = QDateTime(QDate(2100, 12, 31), QTime(23, 59, 59, 999)));
    void setComboBoxForColumn(int column, QAbstractItemModel *model, int modelColumn);
    std::vector<double> getNumericColumn(int);
    template <typename T> void setColumnData(int column, const std::vector<T> &values);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void initHeaderStyles();
};
