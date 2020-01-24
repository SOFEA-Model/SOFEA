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

#include <memory>

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QTabWidget;
class QString;
class QStringList;
QT_END_NAMESPACE

class QtTreePropertyBrowser;
class QtIntPropertyManager;
class QtEnumPropertyManager;
class QtGroupPropertyManager;

struct Project;

class ProjectOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    ProjectOptionsDialog(QWidget *parent = nullptr); // std::shared_ptr<Project> p,
    ~ProjectOptionsDialog();

private slots:
    void accept() override;
    void reject() override;

private:
    //void addUnitsProperty(const QString& name, const QStringList& options, int precision);

    std::shared_ptr<Project> project;

    QTabWidget *tabWidget;
    QtIntPropertyManager *intManager;
    QtEnumPropertyManager *enumManager;
    QtGroupPropertyManager *groupManager;
    QtTreePropertyBrowser *propertyEditor;
    QDialogButtonBox *buttonBox;
};
