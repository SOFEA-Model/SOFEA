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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QMap>

QT_BEGIN_NAMESPACE
class QTextEdit;
class QTreeWidget;
class QTreeWidgetItem;
class QStackedWidget;
QT_END_NAMESPACE

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    void addPage(QString const& label, QWidget *page);

private slots:
    void onFocusChanged(const QWidget *, const QWidget *now);

private:
    QTreeWidget *navTree;
    QMap<QTreeWidgetItem *, QWidget *> navTreeItems;
    QStackedWidget *pageStack;

protected:
    QDialogButtonBox *buttonBox;
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // SETTINGSDIALOG_H
