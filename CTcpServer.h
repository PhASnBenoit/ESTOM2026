// CTcpServer.h

#ifndef CTCP_SERVER_H
#define CTCP_SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

#include "commun.h"

class CTcpServer : public QTcpServer {
    Q_OBJECT
public:
    explicit CTcpServer(QObject *parent = nullptr);
    void startServer(quint16 port = 5005);
    QTcpSocket* getSocketForIP(const QString &ip) const;

signals:
    void infoUpdated(T_INFOS infos, QString ip);

protected:
    void incomingConnection(qintptr socketDescriptor) override;
private:
    QMap<QString, QTcpSocket*> socketMap;
};

#endif // CTCP_SERVER_H
