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

#include <QApplication>
#include <QDialogButtonBox>
#include <QIcon>
#include <QPushButton>
#include <QString>
#include <QTabWidget>
#include <QVBoxLayout>

#include <qtpropertymanager.h>
#include <qteditorfactory.h>
#include <qttreepropertybrowser.h>

#include "AppStyle.h"
#include "ProjectOptionsDialog.h"
#include "Project.h"

ProjectOptionsDialog::ProjectOptionsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Project Options"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_ActionSettings)));

    intManager = new QtIntPropertyManager(this);
    enumManager = new QtEnumPropertyManager(this);
    groupManager = new QtGroupPropertyManager(this);

    QtSpinBoxFactory *spinBoxFactory = new QtSpinBoxFactory(this);
    QtEnumEditorFactory *comboBoxFactory = new QtEnumEditorFactory(this);

    propertyEditor = new QtTreePropertyBrowser();
    propertyEditor->setFactoryForManager(intManager, spinBoxFactory);
    propertyEditor->setFactoryForManager(enumManager, comboBoxFactory);

    // - Time:         hr
    // - Date:         system, user-defined
    // - Distance:     meter, kilometer, foot, yard, mile
    // - Area:         square meter, square foot, acre, hectare, square kilometer, square mile, square yard
    // - Volume:       cubic meter, cubic foot, cubic yard
    // - Velocity:     meter/sec, kilometer/hr, ft/sec, mile/hr
    // - Temperature:  K, °C, °F, °R
    // - Pressure:     pascal, kilopascal, megapascal, psi, bar, atm
    // - Angle:        degree, radian, grad
    // - Lat/Lon:      decimal, DD°MM'SS.SS"
    // - Diffusivity:  cm^2/sec, ...?
    // - Conductivity: s/cm, ...?
    // - Henry's Law:  Pa-m^3/mol, atm-m3/mol, ...?

    tabWidget = new QTabWidget;
    tabWidget->addTab(propertyEditor, tr("Units"));

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget, 1);
    mainLayout->addWidget(buttonBox, 0);

    setLayout(mainLayout);

    connect(buttonBox, &QDialogButtonBox::rejected, this, &ProjectOptionsDialog::reject);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ProjectOptionsDialog::accept);
}

ProjectOptionsDialog::~ProjectOptionsDialog()
{}

void ProjectOptionsDialog::accept()
{
    //editor->save(sPtr);
    QDialog::done(QDialog::Accepted);
}

void ProjectOptionsDialog::reject()
{
    QDialog::done(QDialog::Rejected);
}
