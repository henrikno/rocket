#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "SyncTrack.h"
#include "ServerWrapper.h"

class TrackView;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void sendKey(std::string name, SyncKey key);
    void setStatusMessage(QString msg);
public slots:
    void openFile();
    void runExport();
    void onClientConnected(const QHostAddress &hostAddress);

public:
    Ui::MainWindow *ui;

    TrackView *trackView;
    ServerWrapper *serverWrapper;
};

#endif // MAINWINDOW_H
