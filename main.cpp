#include <QApplication>
#include <QStyleFactory>
#include <QSettings>
#include <QDir>

#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("fusion"));

    // High DPI support.
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QCoreApplication::setOrganizationName("Corteva Agriscience");
    QCoreApplication::setOrganizationDomain("corteva.com");
    QCoreApplication::setApplicationName("SOFEA");

    // Currently only English supported.
    QLocale locale = QLocale(QLocale::English);
    locale.setNumberOptions(QLocale::c().numberOptions());
    QLocale::setDefault(locale);

	// Use INI file instead of registry for QSettings.
    // setDefaultFormat() will be honored when using the default constructor:
    // QSettings::QSettings(QObject *parent = Q_NULLPTR)
    QSettings::setDefaultFormat(QSettings::IniFormat);

    // Set default file paths.
    QSettings settings;
    if (!settings.contains("DefaultDirectory")) {
        QString defaultDirectory = app.applicationDirPath() + QDir::separator() + "examples";
        settings.setValue("DefaultDirectory", defaultDirectory);
    }
    if (!settings.contains("DefaultMetFileDirectory")) {
        QString defaultMetFileDirectory = app.applicationDirPath() + QDir::separator() + "metfiles";
        settings.setValue("DefaultMetFileDirectory", defaultMetFileDirectory);
    }

    MainWindow w;
    w.show();

    return app.exec();
}
