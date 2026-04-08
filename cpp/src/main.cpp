#include <QApplication>
#include <QSettings>
#include "gui/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("PGM-Jenius");
    app.setOrganizationName("PGM-Jenius");
    app.setOrganizationDomain("pgm-jenius.local");

    // Parse args: [program.pgm]
    QString pgm_path;
    for (int i = 1; i < argc; i++) {
        QString arg = argv[i];
        if (!arg.startsWith("-")) {
            pgm_path = arg;
            break;
        }
    }

    MainWindow* window = pgm_path.isEmpty()
        ? new MainWindow()
        : new MainWindow(pgm_path);

    window->show();
    return app.exec();
}
