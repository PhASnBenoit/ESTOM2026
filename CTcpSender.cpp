// CTcpSender.cpp
#include "CTcpSender.h"

CTcpSender::CTcpSender(CTcpServer *tcpServer, QObject *parent) : QObject(parent), _tcpServer(tcpServer) {}

void CTcpSender::on_sendMessage(const QString &ip, int ordre, T_SEND toSend)
{
    qDebug() << "<=====|TCP-SENDER-" << ip <<"|=====>";
    QTcpSocket *socket = _tcpServer->getSocketForIP(ip);

    if (!socket || socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "Socket non disponible ou déconnecté pour l'IP:" << ip;
        return;
    } // if socket

    QJsonObject jsonObj;
    jsonObj["ordre"] = ordre;
    switch(ordre) {
    case 0: // INIT suite à BONJOUR
        qDebug() << "Ordre0: INIT;";
        if (toSend.pb == "B") {
            jsonObj["etat"] = toSend.etatB;    // TODO ALLER CHERCHER DANS LA BDD LES VALEURS EN COURS
            jsonObj["nbChocs"] = toSend.nbChocs;
            jsonObj["leds"] = toSend.leds;
        } else
            jsonObj["etat"] = toSend.etatP;
        break;
    case 1:
        qDebug() << "Ordre1: START;";
        // TODO ajout luminosite
        jsonObj["luminosite"] = toSend.luminosite;
        break;
    case 2: qDebug() << "Ordre2: STOP;";  break;
    case 11: qDebug() << "Ordre11; EMPTYING;"; break;
    case 12: qDebug() << "Ordre12: EMPTY;"; break;
    case 13: qDebug() << "Ordre13: CANCEL;"; break;
    } // sw

    QJsonDocument doc(jsonObj);
    QByteArray message = doc.toJson(QJsonDocument::Compact) + '\n';

    socket->write(message);
    socket->flush();
    qDebug() << "Message TCP envoyé à" << ip;
    qDebug() << "Données:" << message;
}
