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

void ServerWrapper::update()
{
    std::cout << "Update" << std::endl;
    if (clientSocket->connected()) {
        startRead();
    } else {
        acceptConnection();
    }
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

void ServerWrapper::startRead() {

    //std::cout << "startRead ..." << std::endl;

    while (this->clientSocket->pollRead())
        processCommands();

    return;
}

void ServerWrapper::processCommands()
{
    int strLen, serverIndex, newRow;
    std::string trackName;
    unsigned char cmd = 0;
    if (clientSocket->recv((char*)&cmd, 1)) {
        std::cout << "Cmd: " << (int)cmd << std::endl;
        switch (cmd) {
        case GET_TRACK:
            {
            // Get index
            uint32_t index;
            if (!clientSocket->recv((char*)&index, sizeof(index))) {
                std::cerr << "Error" << std::endl;
                return;
            }
            std::cout << "Index: " << index << std::endl;

            // Get track string length
            clientSocket->recv((char *)&strLen, sizeof(int));
            strLen = ntohl(strLen);
            std::cout << "Strlen: " << strLen << std::endl;
            if (!clientSocket->connected())
                return;

            // Get track string
            trackName.resize(strLen);
            if (!clientSocket->recv(&trackName[0], strLen))
                return;
            std::cout << "TrackName: " << trackName << std::endl;

            TrackView *trackView = mainWindow->trackView;
            SyncTrack *track = trackView->getTrack(trackName);
            if (!track) {
                trackView->createTrack(trackName);
                track = trackView->getTrack(trackName);
            }

            clientSocket->clientTracks[trackName] = clientIndex++;

            SyncTrack::iterator it = track->begin();
            for (; it != track->end(); it++) {
                struct track_key key;
                key.row = it->second.row;
                key.value = it->second.value;
                key.type = (key_type)it->second.type;

                clientSocket->sendSetKeyCommand(trackName, key);
                //SendKey(track->GetName(), it->second);
            }

            }
            break;
        case SET_ROW:
            clientSocket->recv((char*)&newRow, sizeof(newRow));
            newRow = ntohl(newRow);
//			trackView->setEditRow(ntohl(newRow));
            emit rowChanged(newRow);
            break;
        default:
            std::cout << "Unknown command: " << (int)cmd << std::endl;
            break;
        }
    }
}

/*
void ServerWrapper::SendKey(std::string name, SyncKey key) {
    //QDataStream stream(client);
    uint8_t cmd = CMD_SET_KEY;
    uint32_t trackIndex = mainWindow->trackView->getTrackIndex(name);
    uint8_t type = key.type;

    union {
            float f;
            uint32_t i;
    } v;
    v.f = key.value;

    stream << cmd;
    stream << trackIndex;
    stream << key.row;
    stream << v.i;
    stream << type;
}*/

void ServerWrapper::ChangeRow(int row)
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

void ServerWrapper::SendPause()
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
