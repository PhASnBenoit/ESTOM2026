// CApp.h
#pragma once // Directive d'inclusion exclusive, plus moderne !

#include <QObject>
#include <QDebug>
#include <QThread>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>

#include "CDatabase.h"
#include "CTcpServer.h"
#include "CTcpSender.h"
#include "CConnectChecker.h"
#include "commun.h"

class CApp : public QObject
{
    Q_OBJECT

public:
    CApp(QObject *parent = nullptr);
    ~CApp();

private slots:
    void on_configUpdated(T_CONFIG cfg);
    void on_infoUpdated(T_INFOS infos, QString ip);

private:
    void init();
    void sendMsgTCP(const QString &ip, int ordre, T_SEND toSend);

    int _currentEtat;
    CDatabase *_dbReader;
    QThread *_connectThread;
    CConnectChecker *_connectChecker;
    QThread *_tcpThread;
    CTcpServer *_tcpServer;
    CTcpSender *_tcpSender;

signals:
    void sig_sendTcpMessage(const QString &ip, int ordre, T_SEND toSend);
};
