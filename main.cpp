#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include <spdlog/spdlog.h>
#include "spdlog/cfg/env.h"   // support for loading levels from the environment variable
#include "spdlog/fmt/ostr.h"  // support for user defined types

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    spdlog::cfg::load_env_levels();

    spdlog::info("start application");

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "test1_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();
    return a.exec();
}
