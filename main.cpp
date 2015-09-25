#include <QApplication>

#include "mainwindow.h"
#include <QDebug>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("Jazz History Database");
    QCoreApplication::setApplicationName("JHDB Dropbox Sorter");
    a.setWindowIcon(QIcon(":/Icons/Icons/Gerald_G_Trumpet_1.svg"));


    MainWindow w;

    return a.exec();
}
