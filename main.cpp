#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QPalette pal(w.palette());
    pal.setColor(QPalette::Window , QColor(176, 196, 222, 100));
    w.setAutoFillBackground(true);
    w.setPalette(pal);
    w.setWindowTitle("五子棋游戏");
    w.show();
    return a.exec();
}
