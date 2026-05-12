// CTcpServer.cpp
#include "CTcpServer.h"

CTcpServer::CTcpServer(QObject *parent) : QTcpServer(parent) {}

void CTcpServer::startServer(quint16 port) {
    if (!this->listen(QHostAddress::Any, port)) {
        qDebug() << "Server TCP n'a pas démarré!";
    } else {
        qDebug() << "Server TCP démarré sur le port" << port;
    } // else
}

QTcpSocket *CTcpServer::getSocketForIP(const QString &ip) const
{
    return socketMap.value(ip, nullptr);
}

void CTcpServer::incomingConnection(qintptr socketDescriptor) {
    QMetaObject::invokeMethod(this, [=]() {
        QTcpSocket *clientSocket = new QTcpSocket(); // Pas de parent ici !

        if (!clientSocket->setSocketDescriptor(socketDescriptor)) {
            qDebug() << "Erreur de socket TCP !";
            clientSocket->deleteLater();
            return;
        } // if client

        // Adresse IP du client connecté
        QString senderIP = clientSocket->peerAddress().toString();
        if (senderIP.startsWith("::ffff:")) {
            senderIP = senderIP.right(senderIP.length() - 7);
        } // if sender
        qDebug() << "Nouvelle connexion TCP de:" << senderIP;

        // Stockage temporaire uniquement si socket est en ConnectedState
        if (clientSocket->state() == QAbstractSocket::ConnectedState) {
            socketMap[senderIP] = clientSocket;
        } else {
            connect(clientSocket, &QTcpSocket::connected, this, [=]() {
                qDebug() << "Connexion confirmée pour:" << senderIP;
                socketMap[senderIP] = clientSocket;
            });
        } // else


        QByteArray *buffer = new QByteArray;
        // Message reçu d'un périphérique (PAV, BOM)
        connect(clientSocket, &QTcpSocket::readyRead, this, [=]() {
            buffer->append(clientSocket->readAll());
            if (!buffer->contains('\n')) return;
            QList<QByteArray> messages = buffer->split('\n');
            for (int i = 0; i < messages.size() - 1; ++i) {
                QByteArray message = messages[i].trimmed();
                if (message.isEmpty()) continue;
                qDebug() << "<=====|TCP-REQUEST-" << senderIP <<"|=====>";
                qDebug() << "Message TCP reçu:" << message;
                QJsonParseError parseError;
                QJsonDocument doc = QJsonDocument::fromJson(message, &parseError);
                if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
                    qWarning() << "Erreur JSON TCP:" << parseError.errorString();
                    continue;
                } // if
                QJsonObject jsonObj = doc.object();
                T_INFOS infos;
                infos.status = jsonObj.value("status").toString();

                switch(infos.status.toInt()) {
                case 0: // BOM/PAV/BUS/ABB annonce BONJOUR
                    infos.type = jsonObj.value("type").toString();
                    infos.couleur = jsonObj.value("couleur").toString();
                    if (infos.type == "BUS")  infos.couleur = "40";
                    if (infos.type == "ABB")  infos.couleur = "40";
                    break;
                case 1: // BOM/BUS annonce DEB TRANSFER
                    infos.type = "BOM";
                    infos.ipPAV = QString(PREFIXE_IP)+jsonObj.value("ipPAV").toString();
                    break;
                case 2: // BOM/Bus annonce FIN TRANSFER
                case 3: // BOM/Bus annonce ANN TRANSFER
                    infos.type = "BOM";
                    infos.ipPAV = QString(PREFIXE_IP)+jsonObj.value("ipPAV").toString();
                    infos.leds = jsonObj.value("leds").toString();
                    break;
                case 4: // BOM/Bus annonce CHOCS
                    infos.type = "BOM";
                    infos.collisions = jsonObj.value("collisions").toString();
                    break;
                case 20: // PAV/ABB VIDE
                    infos.type = "PAV";
                    infos.ipPAV = QString(PREFIXE_IP)+jsonObj.value("ipPAV").toString();
                    break;
                case 21: // PAV/ABB PLEIN
                    infos.type = "PAV";
                    infos.ipPAV = QString(PREFIXE_IP)+jsonObj.value("ipPAV").toString();
                     break;
                case 22: // PAV/ABB VIDAGE
                    infos.type = "PAV";
                    infos.ipPAV = QString(PREFIXE_IP)+jsonObj.value("ipPAV").toString();
                     break;
               case 99: // message debug de BOM
                    qDebug() << jsonObj.value("texte").toString();
                    continue;
                    break;
                } // sw

                // adresse IP du périphérique qui a envoyé la trame
                QString senderIP = clientSocket->peerAddress().toString();
                if (senderIP.startsWith("::ffff:")) {
                    senderIP = senderIP.right(senderIP.length() - 7);
                } // if senderIp

                // Vers la BDD via CApp
                emit sig_infoUpdated(infos, senderIP);
            } // for

            *buffer = messages.last();
        });
        connect(clientSocket, &QTcpSocket::disconnected, clientSocket, &QTcpSocket::deleteLater);
    }, Qt::QueuedConnection);
}
