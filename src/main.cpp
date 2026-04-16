#include "../include/mainwindow.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;

    QFile styleFile(":/qss/styles/defaultstyle.qss");
    styleFile.open(QFile::ReadOnly);
    if (styleFile.isOpen()) {
        QString styleSheet = styleFile.readAll();
        a.setStyleSheet(styleSheet);
    }
    w.show();
    return a.exec();
}
