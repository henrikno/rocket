#ifndef SERVERWRAPPER_H
#define SERVERWRAPPER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include "SyncTrack.h"
#include "clientsocket.h"
class MainWindow;

class ServerWrapper : public QObject
{
    Q_OBJECT
public:
    explicit ServerWrapper(MainWindow *parent = 0);
    ~ServerWrapper();

signals:
    void rowChanged(int row);
    void clientConnected(const QHostAddress &hostAddress);

public slots:
    void acceptConnection();
    void ChangeRow(int row);
    void cellChanged(std::string track, SyncKey key);
    void interpolationTypeChanged(std::string track, SyncKey key2);
    void SendPause();
    void keyDeleted(std::string, SyncKey key);
    void sendExportCommand();

private:
    MainWindow *mainWindow;
    QTcpServer *server;
    int isClientPaused;

    int clientIndex;
    ClientSocket *clientSocket;
};

#endif // SERVERWRAPPER_H
