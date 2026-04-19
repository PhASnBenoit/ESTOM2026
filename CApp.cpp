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

void CApp::on_configUpdated(T_CONFIG cfg)
{
    static int _etatPrecedent = -1; //-1 pour garantir un premier envoi
    _currentEtat = cfg.status.toInt();
    QStringList ipList = _dbReader->getAllIPs();
    T_SEND toSend;

    if (_currentEtat != _etatPrecedent) {
        toSend.etatP = QString::number(_currentEtat);
        for (const QString &ip : ipList) {
            switch (_currentEtat) {
            case 0: // attente départ partie
                if (_dbReader->checkPAV(ip)==true)
                    toSend.pb = "P";
                else
                    toSend.pb = "B";
                sendMsgTCP(ip, 0, toSend);  // Attention, INIT différentselon PAV et BOM
                qDebug() << "Ordre 0, initialisation....";
                break;
            case 1: // départ partie
                sendMsgTCP(ip, 1, toSend);
                qDebug() << "Ordre 1, début de partie....";
                break;
            case 2:  // fin de la partie (podium)
                sendMsgTCP(ip, 2, toSend);
                qDebug() << "Ordre 2, fin de la partie....";
                break;
            case 3: // compte à rebour début partie
                // Pas d'effet sur PAB, BOM
                // sendMsgTCP(ip, 0, type);
                // qDebug() << "Ordre 0....?";
                break;
            } //sw
        } // for
        _etatPrecedent = _currentEtat;
    } // if
} // method

// Appelé lors de la réception d'un état d'un périphérique
// réponse  lorsque c'est nécessaire
void CApp::on_infoUpdated(T_INFOS infos, QString ip)
{
    qDebug() << "<=====|MAJ-INFO-" << ip <<"|=====>";

    T_SEND toSend;
//    toSend.etatJ = QString::number(_currentEtat);
    // états des périphériques BOM ou PAV
    if (infos.type == "BOM") {
        toSend.pb = "B";
        switch (infos.status.toInt()) {
        case 0: //BONJOUR
            qDebug() << "BONJOUR de BOM (" << ip << ")";
            _dbReader->insertDB("BOM", QVariantList{ip, infos.status, infos.couleur});
            sendMsgTCP(ip, 0, toSend); // Message INIT
            break;
        case 1: //EMPTYING
            if (_currentEtat == 1) {
                _dbReader->insertDB("BOM", QVariantList{ip, infos.status});
                qDebug() << "Remplissage en cours de BOM (" << ip << ") qui vide PAV (" << infos.ipPAV << ")";
                sendMsgTCP(infos.ipPAV, 11, toSend);
            } // if
            break;
        case 2: //EMPTY
            if (_currentEtat == 1) {
                _dbReader->insertDB("BOM", QVariantList{ip, infos.status, infos.leds});
                sendMsgTCP(infos.ipPAV, 12, toSend);
            } // if 1
            break;
        case 3: //CANCEL
            if (_currentEtat == 1) {
                _dbReader->insertDB("BOM", QVariantList{ ip, infos.status, infos.leds});
                sendMsgTCP(infos.ipPAV, 13, toSend);
            } // if 1
            break;
        case 4:  //CHOC
            if (_currentEtat == 1) {
                _dbReader->insertDB("BOM", QVariantList{ip, infos.status, infos.collisions});
            } // if 1
            break;
        default:
            qDebug() << "Status BOM inconnu :" << infos.status;
            break;
        } // sw
    } // if BOM

    if (infos.type == "PAV") {
        toSend.pb = "P";
        switch (infos.status.toInt()) {
        case 0: // BONJOUR
            qDebug() << "BONJOUR de PAV (" << ip << ")";
            _dbReader->insertDB("PAV", QVariantList{ip, infos.status, infos.couleur});
            sendMsgTCP(ip, 0, toSend); // envoi de INIT
            break;
        case 20:  // PAV VIDE
            _dbReader->insertDB("PAV", QVariantList{ip, infos.status});
            break;
        case 21: // PAV PLEIN
            _dbReader->insertDB("PAV", QVariantList{ip, infos.status});
            break;
        case 22: // PAV VIDAGE
            _dbReader->insertDB("PAV", QVariantList{ip, infos.status});
            break;
        default:
            qDebug() << "Status BOM inconnu :" << infos.status;
        } // sw
    } // if PAV
}


void CApp::sendMsgTCP(const QString &ip, int ordre, T_SEND toSend)
{
    toSend = _dbReader->getDatasToSend(ip, ordre, toSend);
    emit sig_sendTcpMessage(ip, ordre, toSend);
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

    connect(_tcpServer, &CTcpServer::sig_infoUpdated, this, &CApp::on_infoUpdated);
    // Nettoyage du thread quand il se termine
    connect(_tcpThread, &QThread::finished, _tcpServer, &QObject::deleteLater);
    _tcpThread->start();
    // Création du sender TCP
    _tcpSender = new CTcpSender(_tcpServer);
    _tcpSender->moveToThread(_tcpThread);
    // connecte
    connect(this, &CApp::sig_sendTcpMessage, _tcpSender, &CTcpSender::on_sendTcpMessage);
    // Connexion pour les mises à jour de configuration
    connect(_dbReader, &CDatabase::sig_configUpdated, this, &CApp::on_configUpdated);
    _dbReader->start();

    _connectChecker = new CConnectChecker(_dbReader);
    _connectChecker->start();
}
