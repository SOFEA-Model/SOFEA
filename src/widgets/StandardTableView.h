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
#include <QModelIndexList>
#include <QVariant>

#include <vector>

class StandardTableView : public QTableView
{
    Q_OBJECT

public:
    explicit StandardTableView(QWidget *parent = nullptr);
    void setAutoFilterEnabled(bool enabled);
    QModelIndexList selectedRows() const;

    bool appendRow();
    void selectLastRow();
    bool removeSelectedRows();
    bool moveSelectedRows(int offset);
    void copyClipboard();

    template <typename T>
    std::vector<T> columnData(int column);

    template <typename T>
    void setColumnData(int column, const std::vector<T> &values);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void initHeaderStyles();
    bool scrollOnAppend_ = true;
};
