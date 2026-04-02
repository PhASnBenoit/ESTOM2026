// CTcpSender.h
#pragma once

#include <QTcpSocket>
#include "CTcpServer.h"
#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QHostAddress>
#include <QDebug>
#include "CDatabase.h"

class CTcpSender : public QObject
{
    Q_OBJECT

public:
    explicit CTcpSender(CTcpServer *tcpServer, QObject *parent = nullptr);
    void sendMessage(const QString &ip, int ordre, QString bp);

private:
    CTcpServer *_tcpServer = nullptr;
};
