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
#include <QStandardPaths>
#include <QDir>

#include "AppStyle.h"
#include "MainWindow.h"
#include "core/Common.h"
#include "core/Projection.h"
#include "core/Raster.h"

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

    QCoreApplication::setOrganizationName("Dow AgroSciences");
    QCoreApplication::setOrganizationDomain("dow.com");
    QCoreApplication::setApplicationName("SOFEA");

    QLocale locale = QLocale(QLocale::English);
    locale.setNumberOptions(QLocale::c().numberOptions());
    QLocale::setDefault(locale);

	// Use INI file instead of registry for QSettings.
    // setDefaultFormat() will be honored when using the default constructor:
    // QSettings::QSettings(QObject *parent = Q_NULLPTR)
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QString appPath = app.applicationDirPath();

    // %LOCALAPPDATA%/Dow AgroSciences/SOFEA/cache
    QString appCachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);

    QSettings settings(QSettings::UserScope);
    if (!settings.contains("DefaultDirectory")) {
        QString defaultDirectory = QDir::cleanPath(appPath + QDir::separator() + "examples");
        settings.setValue("DefaultDirectory", defaultDirectory);
    }
    if (!settings.contains("DefaultMetFileDirectory")) {
        QString defaultMetFileDirectory = QDir::cleanPath(appPath + QDir::separator() + "metfiles");
        settings.setValue("DefaultMetFileDirectory", defaultMetFileDirectory);
    }
    if (!settings.contains("DefaultPostFileDirectory")) {
        QString defaultPostFileDirectory = QDir::cleanPath(appPath + QDir::separator() + "examples");
        settings.setValue("DefaultPostFileDirectory", defaultPostFileDirectory);
    }

    QDir::setCurrent(appPath);

    QString projDataPath = QDir::cleanPath(appPath + QDir::separator() + SOFEA_PROJ_DATA_PATH);
    Projection::setSearchPath(projDataPath.toStdString());
    Projection::setCacheDirectory(appCachePath.toStdString());

    QString gdalDataPath = QDir::cleanPath(appPath + QDir::separator() + SOFEA_GDAL_DATA_PATH);
    Raster::setConfigOption("GDAL_DATA", gdalDataPath.toStdString());

    MainWindow w;
    w.setMinimumSize(800, 600);
    w.show();

    return app.exec();
}
