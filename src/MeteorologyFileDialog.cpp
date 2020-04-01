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

#include "MeteorologyFileDialog.h"
#include "widgets/PathEdit.h"
#include "widgets/StatusLabel.h"

#include <QAction>
#include <QBoxLayout>
#include <QButtonGroup>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QSettings>

#include <QDebug>

MeteorologyFileDialog::MeteorologyFileDialog(QWidget *parent)
    : QDialog(parent)
{
    QSettings settings;
    QString defaultDir = settings.value("DefaultMetFileDirectory", QDir::currentPath()).toString();

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    leSurfaceFile = new PathEdit;
    leSurfaceFile->setDialogCaption(tr("Select Surface File"));
    leSurfaceFile->setDialogDirectory(defaultDir);
    leSurfaceFile->setDialogFilter("AERMET Surface Data (*.sfc)");

    leUpperAirFile = new PathEdit;
    leUpperAirFile->setDialogCaption(tr("Select Upper Air File"));
    leUpperAirFile->setDialogDirectory(defaultDir);
    leUpperAirFile->setDialogFilter("AERMET Profile Data (*.pfl)");

    connect(leSurfaceFile, &PathEdit::currentPathChanged,
            this, &MeteorologyFileDialog::onCurrentPathChanged);

    connect(leUpperAirFile, &PathEdit::currentPathChanged,
            this, &MeteorologyFileDialog::onCurrentPathChanged);

    QFormLayout *inputLayout = new QFormLayout;
    inputLayout->addRow(tr("Surface data file:"), leSurfaceFile);
    inputLayout->addRow(tr("Upper air data file:"), leUpperAirFile);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    connect(buttonBox, &QDialogButtonBox::rejected, this, &MeteorologyFileDialog::reject);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &MeteorologyFileDialog::accept);
}

void MeteorologyFileDialog::onCurrentPathChanged(const QString& path)
{
    if (path.isEmpty())
        return;

    QFileInfo fi(path);
    QString defaultDir = fi.absoluteDir().absolutePath();
    QSettings settings;
    settings.setValue("DefaultMetFileDirectory", defaultDir);

    leUpperAirFile->setDialogDirectory(defaultDir);
    leSurfaceFile->setDialogDirectory(defaultDir);
}

void MeteorologyFileDialog::accept()
{
    QDialog::done(QDialog::Accepted);
}

void MeteorologyFileDialog::reject()
{
    QDialog::done(QDialog::Rejected);
}
