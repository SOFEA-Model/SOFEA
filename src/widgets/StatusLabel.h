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

#include <QWidget>
#include <QString>

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

class StatusLabel : public QWidget
{
public:
    StatusLabel(QWidget *parent = nullptr);

    enum StatusType {
        Help,
        OK,
        InfoTip,
        Required,
        Debug,
        Information,
        Alert,
        Warning,
        Invalid,
        Critical
    };

    void setStatusType(StatusType type);
    void setText(const QString& text);
    QLabel * label();

private:
    QLabel *m_iconLabel;
    QLabel *m_textLabel;
};
