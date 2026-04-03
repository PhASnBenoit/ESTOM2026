// CTcpServer.h
#pragma once

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
    void startServer(quint16 port = TCP_PORT);
    QTcpSocket* getSocketForIP(const QString &ip) const;

signals:
    void sig_infoUpdated(T_INFOS infos, QString ip);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    QMap<QString, QTcpSocket*> socketMap;
};
