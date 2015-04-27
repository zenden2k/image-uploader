#include "Gui/mainwindow.h"
#include <QApplication>
//#include <3rdparty/qtdotnetstyle.h>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //QApplication::setStyle(new QtDotNetStyle);
    MainWindow w;
    w.show();
    
    return a.exec();
}
