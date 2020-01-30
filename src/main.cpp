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
#include <QLoggingCategory>
#include <QSettings>
#include <QDir>

#include "MainWindow.h"
#include "AppStyle.h"
#include "Projection.h"
#include "UDUnitsInterface.h"

int main(int argc, char *argv[])
{
    // Use native high DPI support.
    QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);

    QApplication app(argc, argv);
    app.setStyle(new AppStyle);
    //app.setPalette(app.style()->standardPalette());

    QFont font = app.font();
    font.setFamily("Segoe UI");
    font.setPointSize(font.pointSize() + 1);
    app.setFont(font);

    QCoreApplication::setOrganizationName("Dow");
    QCoreApplication::setOrganizationDomain("dow.com");
    QCoreApplication::setApplicationName("SOFEA");

    QLocale locale = QLocale(QLocale::English);
    locale.setNumberOptions(QLocale::c().numberOptions());
    QLocale::setDefault(locale);

	// Use INI file instead of registry for QSettings.
    // setDefaultFormat() will be honored when using the default constructor:
    // QSettings::QSettings(QObject *parent = Q_NULLPTR)
    QSettings::setDefaultFormat(QSettings::IniFormat);

    // Set default file paths.
    QString appPath = app.applicationDirPath();
    QSettings settings;
    if (!settings.contains("DefaultDirectory")) {
        QString defaultDirectory = appPath + QDir::separator() + "examples";
        settings.setValue("DefaultDirectory", defaultDirectory);
    }
    if (!settings.contains("DefaultMetFileDirectory")) {
        QString defaultMetFileDirectory = appPath + QDir::separator() + "metfiles";
        settings.setValue("DefaultMetFileDirectory", defaultMetFileDirectory);
    }

    QString dbPath = QDir::cleanPath(appPath + QDir::separator() + "share/proj/proj.db");
    Projection::Context::setDatabasePath(dbPath.toStdString());

    MainWindow w;
    w.setMinimumSize(800, 600);
    w.show();

    return app.exec();
}
