#ifndef CAPP_H
#define CAPP_H

#include <QObject>
#include <QDebug>
#include <QThread>

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
    CDatabase *_dbReader;
    QThread *_connectThread;
    CConnectChecker *_connectChecker;
    QThread *_tcpThread;

    CTcpServer *_tcpServer;
    CTcpSender *_tcpSender;
    void sendMsgTCP(const QString &ip, int ordre, const QString &extraData ="");

    //Config variable
    QString _options;
    QString _ptsRecolte;
    QString _nbrPAV;
    QString _etatGeneral;
    QString _luminosite;

    //Info variable
    QString _ip; // IP du périph envoyant un état
    T_INFOS _infos;
    QString _stockage;
    QString _state;
    QString _couleur;
    QString _progression;
    QString _nbrCollision;
    QString _ipPAV;

    QString _connected;

    bool _messageEnvoye = false;
signals:
    void sendTcpMessageRequested(const QString &ip, int ordre, const QString &extraData ="");
};


#endif // CAPP_H
