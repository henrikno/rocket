#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QTimer>
#include "TrackView.h"
#include <iostream>
#include "SyncTrack.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    trackView = new TrackView(this);
    setCentralWidget(trackView);

    setStatusMessage("Not connected");

    serverWrapper = new ServerWrapper(this);

    connect(trackView, SIGNAL(rowChanged(int)), serverWrapper, SLOT(changeRow(int)));
    connect(serverWrapper, SIGNAL(rowChanged(int)), trackView, SLOT(changeRow(int)));
    connect(serverWrapper, SIGNAL(clientConnected(const QHostAddress &)), this, SLOT(onClientConnected(const QHostAddress &)));
    connect(trackView, SIGNAL(cellChanged(std::string,SyncKey)), serverWrapper, SLOT(cellChanged(std::string,SyncKey)));
    connect(trackView, SIGNAL(interpolationTypeChanged(std::string,SyncKey)), serverWrapper, SLOT(interpolationTypeChanged(std::string,SyncKey)));
    connect(trackView, SIGNAL(pauseTriggered()), serverWrapper, SLOT(sendPause()));
    connect(trackView, SIGNAL(deleteKey(std::string,SyncKey)), serverWrapper, SLOT(keyDeleted(std::string,SyncKey)));
}

void MainWindow::setStatusMessage(QString msg) {
    ui->statusBar->showMessage(msg);
}

MainWindow::~MainWindow()
{
    delete serverWrapper;
    delete ui;
}

void MainWindow::onClientConnected(const QHostAddress &hostAddress)
{
    setStatusMessage(QString("Connected to ") + hostAddress.toString());
    serverWrapper->changeRow(trackView->getCurrentRow());
}

void MainWindow::openFile()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Directory"));
    if (path.isNull() == false)
    {
        std::cout << "Not implemented" << std::endl;
    }
}

void MainWindow::runExport()
{
    serverWrapper->sendExportCommand();
}

