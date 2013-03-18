#include <iostream>
#include "ServerWrapper.h"
#include "TrackView.h"
#include "MainWindow.h"
#include <cassert>
#include <fcntl.h>
#ifndef WIN32
#include <unistd.h>
#else
#define socklen_t int
#endif
#include "../sync/base.h"
#include <stdexcept>
#include "../sync/track.h"

const int SYNC_PORT = 1338;

ServerWrapper::ServerWrapper(MainWindow *parent) :
    QObject((QObject*)parent), mainWindow(parent)
{
    isClientPaused = true;
    server = new QTcpServer(this);
    if (!server->listen(QHostAddress::Any, SYNC_PORT))
        throw std::runtime_error("Could not listen on port");
    std::cout << "Listening on port " << SYNC_PORT << std::endl;
    connect(server, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
}

ServerWrapper::~ServerWrapper() {
}

void ServerWrapper::acceptConnection()
{
    std::cout << "acceptConnection" << std::endl;
    if (server->hasPendingConnections()) {
        QTcpSocket *clientSocket = server->nextPendingConnection();
        this->clientSocket = new ClientSocket(clientSocket, mainWindow);
        connect(this->clientSocket, SIGNAL(rowChanged(int)), this, SIGNAL(rowChanged(int)));
        connect(this->clientSocket, SIGNAL(clientConnected(const QHostAddress &)), this, SIGNAL(clientConnected(const QHostAddress &)));

        isClientPaused = true;
    }
}

void ServerWrapper::changeRow(int row)
{
    if (isClientPaused) {
        clientSocket->sendSetRowCommand(row);
    }
}

void ServerWrapper::cellChanged(std::string track, SyncKey key2)
{
    std::cout << "cell changed" << std::endl;
    struct track_key key;
    key.row = key2.row;
    key.value = key2.value;
    key.type = (key_type)key2.type;
    clientSocket->sendSetKeyCommand(track, key);
}

void ServerWrapper::interpolationTypeChanged(std::string track, SyncKey key2)
{
    struct track_key key;
    key.row = key2.row;
    key.value = key2.value;
    key.type = (key_type)key2.type;
    clientSocket->sendSetKeyCommand(track, key);
}

void ServerWrapper::sendPause()
{
    clientSocket->sendPauseCommand(!isClientPaused);
    isClientPaused = !isClientPaused;
}

void ServerWrapper::keyDeleted(std::string track, SyncKey key)
{
    clientSocket->sendDeleteKeyCommand(track, key.row);
}

void ServerWrapper::sendExportCommand()
{
    clientSocket->sendSaveCommand();
}
