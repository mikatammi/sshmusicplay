#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class ConnectWindow;
}

class ConnectWindow : public QMainWindow
{
    Q_OBJECT
public:
    enum ScreenOrientation {
        ScreenOrientationLockPortrait,
        ScreenOrientationLockLandscape,
        ScreenOrientationAuto
    };

    explicit ConnectWindow(QWidget *parent = 0);
    virtual ~ConnectWindow();

    // Note that this will only have an effect on Fremantle.
    void setOrientation(ScreenOrientation orientation);

    void showExpanded();

private slots:
    void doConnect();

private:
    Ui::ConnectWindow *ui;
};

#endif // MAINWINDOW_H
