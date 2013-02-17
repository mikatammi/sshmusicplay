#include "connectwindow.hpp"

#include <QApplication>
#include <QDebug>

extern "C"
{
#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ConnectWindow connectWindow;
    connectWindow.setOrientation(ConnectWindow::ScreenOrientationAuto);
    connectWindow.showExpanded();

    qDebug() << "Initializing libavcodec/libavformat";

    // Register all formats and codecs for libavcodec/libavformat
    av_register_all();

    int retval = app.exec();

    return retval;
}
