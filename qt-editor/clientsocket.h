#pragma once
#include "../sync/base.h"
#include <map>
#include <string>
#include <QTcpSocket>
#include <iostream>
#include <QObject>

class MainWindow;

enum ClientState {
    WAITING,
    VERIFYING,
    READY,
    CLIENT_GET_TRACK,
    CLIENT_GET_TRACK_STRING,
    CLIENT_SET_ROW,
    STATE_ERROR
};

class ClientSocket : public QObject
{
    Q_OBJECT
public:
    virtual ~ClientSocket() {}

    explicit ClientSocket(QAbstractSocket *socket, MainWindow *parent) : clientPaused(true), socket(socket), state(VERIFYING), mainWindow(parent), clientIndex(0) {
        connect(socket, SIGNAL(readyRead()), this, SLOT(readMessage()));
        connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    }

	bool connected() const
	{
		return socket != NULL;
	}

	void disconnect()
	{
		socket = NULL;
		clientTracks.clear();
	}

    bool recv(char *buffer, qint64 length)
	{
        if (!connected()) {
            std::cout << "Wasn't connected, yo" << std::endl;
			return false;
        }
        qint64 ret = socket->read(buffer, length);
		if (ret != length) {
            std::cout << "Not enough bytes. Expected: " << length << ", was: " << ret << std::endl;
			disconnect();
			return false;
		}
		return true;
	}

    bool send(const char *buffer, qint64 length)
	{
        if (!connected())
			return false;
		qint64 ret = socket->write(buffer, length);
		if (ret != length) {
			disconnect();
			return false;
		}
		return true;
	}

	bool pollRead()
	{
		if (!connected())
			return false;
		return socket->bytesAvailable() > 0;
	}

	void sendSetKeyCommand(const std::string &trackName, const struct track_key &key);
	void sendDeleteKeyCommand(const std::string &trackName, int row);
	void sendSetRowCommand(int row);
	void sendPauseCommand(bool pause);
	void sendSaveCommand();

	bool clientPaused;
	std::map<const std::string, size_t> clientTracks;

signals:
    void rowChanged(int row);
    void clientConnected(const QHostAddress &hostAddress);

private slots:
    void readMessage();
    void onDisconnected();

private:
	QAbstractSocket *socket;
    ClientState state;
    MainWindow *mainWindow;
    int clientIndex;

    uint32_t tmpIndex;
    int tmpStrLen;

};
