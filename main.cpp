#include <QApplication>

#include "mainwindow.h"
#include <QDebug>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("My Company");
    QCoreApplication::setApplicationName("Back-up Utility");
    a.setWindowIcon(QIcon(":/Icons/Icons/File Cabinet/1434274439_Card_file.png"));


    MainWindow w;

    return a.exec();
}
