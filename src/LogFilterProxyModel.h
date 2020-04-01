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

#include <QHash>
#include <QSortFilterProxyModel>
#include <QString>

class LogFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    LogFilterProxyModel(QObject *parent = nullptr);
    void setTagVisible(const QString& tag, bool visible);
    void setErrorsVisible(bool visible);
    void setWarningsVisible(bool visible);
    void setMessagesVisible(bool visible);

protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override;

private:
    QHash<QString, bool> tagVisibility;
    bool errorsVisible = true;
    bool warningsVisible = true;
    bool messagesVisible = true;
};
