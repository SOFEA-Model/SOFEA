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
#include <QStyle>

QT_BEGIN_NAMESPACE
class QMenu;
class QMouseEvent;
class QPaintEvent;
class QWidget;
QT_END_NAMESPACE

class MenuLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit MenuLineEdit(QWidget *parent = nullptr);

    void setMenu(QMenu *menu);
    QMenu * menu();

public slots:
    void showMenu();

protected:
    virtual void paintEvent(QPaintEvent *e) override;
    virtual void mousePressEvent(QMouseEvent *e) override;

private:
    void updateTextMargins();

    QStyle::StateFlag arrowState_ = QStyle::State_None;
    QMenu *menu_ = nullptr;
};
