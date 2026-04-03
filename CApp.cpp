#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include "CApp.h"

CApp::CApp(QObject *parent)
    : QObject(parent)
{
    init();
}

CApp::~CApp()
{
    if (_dbReader) {
        _dbReader->requestInterruption();
        _dbReader->wait();
        delete _dbReader;
    }
    if (_tcpThread) {
        _tcpThread->quit();
        _tcpThread->wait();
        delete _tcpThread;
    }
    delete _tcpSender;
    if (_connectThread) {
        _connectThread->quit();
        _connectThread->wait();
        delete _connectThread;
    }
    delete _connectChecker;
}

void CApp::onConfigUpdated(T_CONFIG cfg)
{
    static int _etatPrecedent = -1; //-1 pour garantir un premier envoi
    int currentEtat = cfg.status.toInt();
    QStringList ipList = _dbReader->getAllIPs();

    if (currentEtat != _etatPrecedent) {
        for (const QString &ip : ipList) {
            switch (currentEtat) {
            case 0: // attente départ partie
                // L'ordre 0 répond seulement à la trame bonjour
                // sendMsgTCP(ip, 0, type);
                // qDebug() << "Ordre 0, initialisation....";
                break;
            case 1: // départ partie
                sendMsgTCP(ip, 1);
                qDebug() << "Ordre 1, début de partie....";
                break;
            case 2:  // fin de la partie (podium)
                sendMsgTCP(ip, 2);
                qDebug() << "Ordre 2, fin de la partie....";
                break;
            case 3: // compte à rebour début partie
                // Pas d'effet sur PAB, BOM
                // sendMsgTCP(ip, 0, type);
                // qDebug() << "Ordre 0....?";
                break;
            } //sw
        } // for
        _etatPrecedent = currentEtat;
    } // if
} // method


void CApp::onInfoUpdated(T_INFOS infos, QString ip)
{
    qDebug() << "<=====|MAJ-INFO-" << ip <<"|=====>";

    // états des périphériques BOM ou PAV
    if (infos.type == "BOM") {
        switch (infos.status.toInt()) {
        case 0: //BONJOUR
            _dbReader->insertDB("BOM", QVariantList{ip, infos.couleur, infos.status});
            qDebug() << "BONJOUR de BOM (" << ip << ")";
            // TODO Récupérer BDD et Envoyer INIT au BOM
            break;
        case 1: //EMPTYING
            _dbReader->insertDB("BOM", QVariantList{ip, infos.status});
            qDebug() << "Remplissage en cours de BOM (" << ip << ") qui vide PAV (" << infos.ipPAV << ")";
            sendMsgTCP(infos.ipPAV, 11, "B");
            break;
        case 2: //EMPTY
            _dbReader->insertDB("BOM", QVariantList{ip, infos.status, infos.leds});
            sendMsgTCP(infos.ipPAV, 12, "B");
            break;
        case 3: //CANCEL
            _dbReader->insertDB("BOM", QVariantList{ ip, infos.status, infos.leds});
            sendMsgTCP(infos.ipPAV, 13, "B");
            break;
        case 4:  //CHOC
            _dbReader->insertDB("BOM", QVariantList{ip, infos.status, infos.nbCollisions});
            break;
        default:
            qDebug() << "Status BOM inconnu :" << infos.status;
            break;
        } // sw
    } // if BOM

    if (infos.type == "PAV") {
        // une seule trame possible
        if (infos.status.toInt() == 0) {  // BONJOUR
            _dbReader->insertDB("PAV", QVariantList{ip, infos.status, infos.couleur});
            // TODO Récupérer BDD et émettre trame INIT
        } else
            qDebug() << "Status BOM inconnu :" << infos.status;
    } // if PAV
}

void CApp::init() {
    // Initialisation de la base de données
    _dbReader = new CDatabase(this);

    // Démarrage du serveur TCP dans un thread
    _tcpServer = new CTcpServer;
    _tcpThread = new QThread();
    _tcpServer->moveToThread(_tcpThread);

    // Connexion du signal de démarrage
    connect(_tcpThread, &QThread::started, [=]() {
        _tcpServer->startServer(TCP_PORT);
    });

    connect(_tcpServer, &CTcpServer::infoUpdated, this, &CApp::onInfoUpdated);
    // Nettoyage du thread quand il se termine
    connect(_tcpThread, &QThread::finished, _tcpServer, &QObject::deleteLater);
    _tcpThread->start();
    // Création du sender TCP
    _tcpSender = new CTcpSender(_tcpServer);
    _tcpSender->moveToThread(_tcpThread);
    // connecte
    connect(this, &CApp::sendTcpMessageRequested, _tcpSender, &CTcpSender::on_sendMessage);
    // Connexion pour les mises à jour de configuration
    connect(_dbReader, &CDatabase::configUpdated, this, &CApp::onConfigUpdated);
    _dbReader->start();

    _connectChecker = new CConnectChecker(_dbReader);
    _connectChecker->start();
}

void CApp::sendMsgTCP(const QString &ip, int ordre, const QString &extraData)
{
    emit sendTcpMessageRequested(ip, ordre, extraData);
}
