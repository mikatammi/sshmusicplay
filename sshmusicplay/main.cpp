#include "mainwindow.hpp"
#include "libav.hpp"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow mainWindow;
    mainWindow.setOrientation(MainWindow::ScreenOrientationAuto);
    mainWindow.showExpanded();

    qDebug() << "Initializing libavcodec/libavformat";

    // Register all formats and codecs for libavcodec/libavformat
    av_register_all();

    int retval = app.exec();

    return retval;
}
