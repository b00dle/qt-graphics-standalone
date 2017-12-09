#include "mainwindow.h"
#include <QApplication>
#include "lib.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Resources::Lib::init();

    MainWindow w;
    w.show();
    int ret_code = a.exec();

    Resources::Lib::cleanup();

    return ret_code;
}
