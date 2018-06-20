#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QStringList paths = QCoreApplication::libraryPaths();
#ifdef _WIN32
    paths.append(".");
    paths.append("imageformats");
    paths.append("platforms");
#endif
    QCoreApplication::setLibraryPaths(paths);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
