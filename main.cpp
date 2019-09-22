#include <QApplication>
#include <QPalette>
#include <QStyleFactory>
#include <QSettings>
#include <QDir>

#include "MainWindow.h"
#include "AppStyle.h"
#include "Projection.h"
#include "UDUnitsInterface.h"

int main(int argc, char *argv[])
{
    // High DPI support.
    QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
    //QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);
    app.setStyle(new AppStyle);

    QFont font = app.font();
    font.setFamily("Segoe UI");
    font.setPointSize(font.pointSize() + 1);
    app.setFont(font);

    //QPalette darkPalette;
    //darkPalette.setColor(QPalette::Window, QColor(53,53,53));
    //darkPalette.setColor(QPalette::WindowText, Qt::white);
    //darkPalette.setColor(QPalette::Base, QColor(42,42,42));
    //darkPalette.setColor(QPalette::AlternateBase, QColor(66,66,66));
    //darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    //darkPalette.setColor(QPalette::ToolTipText, QColor(53,53,53));
    //darkPalette.setColor(QPalette::Text, Qt::white);
    //darkPalette.setColor(QPalette::Light, QColor(71,71,71));
    //darkPalette.setColor(QPalette::Midlight, QColor(62,62,62));
    //darkPalette.setColor(QPalette::Dark, QColor(35,35,35));
    //darkPalette.setColor(QPalette::Mid, QColor(44,44,44));
    //darkPalette.setColor(QPalette::Shadow, QColor(20,20,20));
    //darkPalette.setColor(QPalette::Button, QColor(53,53,53));
    //darkPalette.setColor(QPalette::ButtonText, Qt::white);
    //darkPalette.setColor(QPalette::BrightText, Qt::red);
    //darkPalette.setColor(QPalette::Link, QColor(42,130,218));
    //darkPalette.setColor(QPalette::Highlight, QColor(42,130,218));
    //darkPalette.setColor(QPalette::HighlightedText, Qt::white);
    //darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127,127,127));
    //darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127,127,127));
    //darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127,127,127));
    //darkPalette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80,80,80));
    //darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127,127,127));
    //app.setPalette(darkPalette);

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
    w.show();

    return app.exec();
}
