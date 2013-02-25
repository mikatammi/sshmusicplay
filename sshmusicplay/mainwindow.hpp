#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItem>
#include <QStandardItemModel>
#include <tr1/memory>

#include "sshsession.hpp"
#include "sshfile.hpp"
#include "audiofile.hpp"
#include "audiooutput.hpp"
#include "downloadbuffer.hpp"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    enum ScreenOrientation {
        ScreenOrientationLockPortrait,
        ScreenOrientationLockLandscape,
        ScreenOrientationAuto
    };

    explicit MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

    // Note that this will only have an effect on Fremantle.
    void setOrientation(ScreenOrientation orientation);

    void showExpanded();

private slots:
    void doPlayPause();

    void doConnect();

private:
    Ui::MainWindow *ui_;

    SSHSession sshsession_;
    std::tr1::shared_ptr <SSHFile> sshfile_;

    QStandardItemModel filelist_model_;

    std::tr1::shared_ptr <DownloadBuffer> downloadbuffer_;
    std::tr1::shared_ptr <AudioFile> audiofile_;
    std::tr1::shared_ptr <AudioOutput> audiooutput_;
};

#endif // MAINWINDOW_H
