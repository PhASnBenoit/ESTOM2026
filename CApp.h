#ifndef CAPP_H
#define CAPP_H

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
    void onConfigUpdated(T_CONFIG cfg);
    void onInfoUpdated(T_INFOS infos, QString ip);

private:
    void init();
    void sendMsgTCP(const QString &ip, int ordre, T_SEND toSend);

    int _luminosite; // PhA à vérifier utilité
    CDatabase *_dbReader;
    QThread *_connectThread;
    CConnectChecker *_connectChecker;
    QThread *_tcpThread;
    CTcpServer *_tcpServer;
    CTcpSender *_tcpSender;

signals:
    void sig_sendTcpMessageRequested(const QString &ip, int ordre, T_SEND toSend);
};


#endif // CAPP_H
