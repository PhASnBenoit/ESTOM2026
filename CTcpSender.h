// CTcpSender.h
#pragma once

#include <QTcpSocket>
#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QHostAddress>
#include <QDebug>

#include "CTcpServer.h"
#include "CDatabase.h"
#include "commun.h"

class CTcpSender : public QObject
{
    Q_OBJECT

public:
    explicit CTcpSender(CTcpServer *tcpServer, QObject *parent = nullptr);
    void on_sendTcpMessage(const QString &ip, int ordre, T_SEND toSend);

private:
    CTcpServer *_tcpServer = nullptr;
};
