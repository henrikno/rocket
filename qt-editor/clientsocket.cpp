#include "clientsocket.h"
#include "../sync/track.h"
#include "TrackView.h"
#include "MainWindow.h"

#include <cassert>
#include <string>
#include <iostream>

void ClientSocket::sendSetKeyCommand(const std::string &trackName, const struct track_key &key)
{
    if (!connected() || clientTracks.count(trackName) == 0)
		return;
	uint32_t track = htonl(clientTracks[trackName]);
	uint32_t row = htonl(key.row);

	union {
		float f;
		uint32_t i;
	} v;
	v.f = key.value;
	v.i = htonl(v.i);

	assert(key.type < KEY_TYPE_COUNT);

	unsigned char cmd = SET_KEY;
	send((char *)&cmd, 1);
	send((char *)&track, sizeof(track));
	send((char *)&row, sizeof(row));
	send((char *)&v.i, sizeof(v.i));
	send((char *)&key.type, 1);
}

void ClientSocket::sendDeleteKeyCommand(const std::string &trackName, int row)
{
	if (!connected() ||
	    clientTracks.count(trackName) == 0)
		return;

	uint32_t track = htonl(int(clientTracks[trackName]));
	row = htonl(row);

	unsigned char cmd = DELETE_KEY;
	send((char *)&cmd, 1);
	send((char *)&track, sizeof(int));
	send((char *)&row,   sizeof(int));
}

void ClientSocket::sendSetRowCommand(int row)
{
	if (!connected())
		return;

	unsigned char cmd = SET_ROW;
	row = htonl(row);
	send((char *)&cmd, 1);
	send((char *)&row, sizeof(int));
}

void ClientSocket::sendPauseCommand(bool pause)
{
	if (!connected())
		return;

	unsigned char cmd = PAUSE, flag = pause;
	send((char *)&cmd, 1);
	send((char *)&flag, 1);
	clientPaused = pause;
}

void ClientSocket::sendSaveCommand()
{
    std::cout << "export" << std::endl;
	if (!connected())
		return;

	unsigned char cmd = SAVE_TRACKS;
    send((char *)&cmd, 1);
}

void ClientSocket::readMessage()
{
    while (socket->bytesAvailable()) {
        std::cout << "Current state: " << state << std::endl;

        switch (state) {
        case VERIFYING:
        {
            if (socket->bytesAvailable() >= strlen(CLIENT_GREET)) {
                const char *expectedGreeting = CLIENT_GREET;
                char recievedGreeting[128];

                qint64 len = socket->read(recievedGreeting, strlen(expectedGreeting));
                std::cout << "len: " << len << std::endl;
                if (strncmp(expectedGreeting, recievedGreeting, strlen(expectedGreeting)) != 0) {
                    std::cerr << "Expected greet, got " << recievedGreeting << std::endl;
                    state = STATE_ERROR;
                    return;
                }
                std::cout << "Greeting was ok" << std::endl;

                const char *greeting = SERVER_GREET;
                socket->write(greeting, strlen(greeting));
                sendPauseCommand(true);

                state = READY;
                emit clientConnected(socket->peerAddress());
            } else {
                return;
            }

        }
            break;

        case READY:
            {
                std::cout << "Ready" << std::endl;
                unsigned char cmd = 0;
                if (socket->bytesAvailable() >= 1) {
                    if (recv((char*)&cmd, 1)) {
                        switch (cmd) {
                            case GET_TRACK:
                                state = CLIENT_GET_TRACK;
                                break;
                            case SET_ROW:
                                state = CLIENT_SET_ROW;
                                break;
                            default:
                                std::cout << "Unknown command" << cmd << std::endl;
                                break;
                        }
                    }
                } else {
                    return;
                }
            }
            break;
        case CLIENT_GET_TRACK:
            std::cout << "GetTrack" << std::endl;

            if (socket->bytesAvailable() >= sizeof(tmpIndex) + sizeof(tmpStrLen)) {

                // Get index
                if (!recv((char*)&tmpIndex, sizeof(tmpIndex))) {
                    std::cerr << "Error" << std::endl;
                    return;
                }
                std::cout << "Index: " << tmpIndex << std::endl;

                // Get track string length

                recv((char *)&tmpStrLen, sizeof(tmpStrLen));
                tmpStrLen = ntohl(tmpStrLen);
                std::cout << "Strlen: " << tmpStrLen << std::endl;
                if (!connected())
                    return;

                state = CLIENT_GET_TRACK_STRING;
            } else {
                return;
            }
            break;

        case CLIENT_GET_TRACK_STRING:
            if (socket->bytesAvailable() >= tmpStrLen) {
                // Get track string
                std::string trackName;
                trackName.resize(tmpStrLen);
                if (!recv(&trackName[0], tmpStrLen))
                    return;
                std::cout << "TrackName: " << trackName << std::endl;

                TrackView *trackView = mainWindow->trackView;
                SyncTrack *track = trackView->getTrack(trackName);
                if (!track) {
                    trackView->createTrack(trackName);
                    track = trackView->getTrack(trackName);
                }

                clientTracks[trackName] = clientIndex++;

                SyncTrack::iterator it = track->begin();
                for (; it != track->end(); it++) {
                    struct track_key key;
                    key.row = it->second.row;
                    key.value = it->second.value;
                    key.type = (key_type)it->second.type;

                    sendSetKeyCommand(trackName, key);
                    //SendKey(track->GetName(), it->second);
                }
                state = READY;
            } else {
                return;
            }
            std::cout << "ok " << socket->bytesAvailable() << std::endl;

            break;

        case CLIENT_SET_ROW:
        {
            int newRow;
            std::cout << "SetRow" << std::endl;

            if (socket->bytesAvailable() >= sizeof(newRow)) {
                recv((char*)&newRow, sizeof(newRow));
                newRow = ntohl(newRow);
                emit rowChanged(newRow);
                state = READY;
            } else {
                return;
            }
            break;
        }

        default:
            std::cout << "Unknown state" << std::endl;
        }
    }
}

void ClientSocket::onDisconnected()
{
    std::cout << "Disconnected" << std::endl;
    state = WAITING;
}
