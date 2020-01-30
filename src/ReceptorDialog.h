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

#include <QDialog>
#include <QWidget>

class ReceptorEditor;
struct Scenario;

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QKeyEvent;
class QShowEvent;
QT_END_NAMESPACE

class ReceptorDialog : public QDialog
{
    Q_OBJECT

public:
    ReceptorDialog(Scenario *s, QWidget *parent = nullptr);
    ~ReceptorDialog();

private slots:
    void accept() override;
    void reject() override;

private:
    Scenario *sPtr;
    ReceptorEditor *editor;
    QDialogButtonBox *buttonBox;

protected:
    void keyPressEvent(QKeyEvent *event) override;
};
