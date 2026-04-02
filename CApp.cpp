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
    this->_etatGeneral = cfg.status;  //0=init/stopped; 1=running; 2=stopping; 3=loading (countdown)
    this->_options = cfg.options;
    this->_ptsRecolte = cfg.ptsRecolte;
    this->_nbrPAV = cfg.nbrPAV;
    this->_luminosite = cfg.luminosite;

    int currentEtat = _etatGeneral.toInt();
    QStringList ipList = _dbReader->getAllIPs();
    //QString type;

    if (currentEtat != _etatPrecedent) {
        for (const QString &ip : ipList) {
            //type = _dbReader->whoAmI(ip);
            switch (currentEtat) {
            case 0: // attente départ partie
                // L'ordre 0 répond à la trame bonjour
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
    _ip = ip;
    _infos.type = infos.type;
    _infos.status = infos.status; // état du périph
    _infos.couleur = infos.couleur; //
    _infos.nbCollisions = infos.nbCollisions;
    _infos.ipPAV = infos.ipPAV;
    //_progression = progression; // Correspond à leds du BOM

    qDebug() << "<=====|MAJ-INFO-" << _ip <<"|=====>";

    // états des périphériques BOM ou PAV
    if (_infos.type == "BOM") {
        switch (_infos.status.toInt()) {
        case 0: //BONJOUR
            _dbReader->insertDB("BOM", QVariantList{_ip, _infos.couleur, _infos.status});
            qDebug() << "BONJOUR de BOM (" << _ip << ")";
            // TODO Récupérer BDD et Envoyer INIT au BOM
            break;
        case 1: //EMPTYING
            _dbReader->insertDB("BOM", QVariantList{_ip, _infos.status});
            qDebug() << "Remplissage en cours de BOM (" << _ip << ") qui vide PAV (" << _ipPAV << ")";
            sendMsgTCP(_ipPAV, 11, "B");
            break;
        case 2: //EMPTY
            _dbReader->insertDB("BOM", QVariantList{_ip, _infos.status, _infos.leds});
            sendMsgTCP(_ipPAV, 12, "B");
            break;
        case 3: //CANCEL
            _dbReader->insertDB("BOM", QVariantList{ _ip, _infos.status, _infos.leds});
            sendMsgTCP(_ipPAV, 13, "B");
            break;
        case 4:  //CHOC
            _dbReader->insertDB("BOM", QVariantList{_ip, _infos.status, _infos.nbCollisions});
            break;
        default:
            qDebug() << "Status BOM inconnu :" << _state;
            break;
        } // sw
    } // if BOM

    if (_infos.type == "PAV") {
        // une seule trame possible
        if (_infos.status.toInt() == 0) {  // BONJOUR
            _dbReader->insertDB("PAV", QVariantList{_ip, _infos.status, _infos.couleur});
            // TODO Récupérer BDD et émettre trame INIT
        } else
            qDebug() << "Status BOM inconnu :" << _state;
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
        _tcpServer->startServer(5005);
    });

    connect(_tcpServer, &CTcpServer::infoUpdated, this, &CApp::onInfoUpdated);
    // Nettoyage du thread quand il se termine
    connect(_tcpThread, &QThread::finished, _tcpServer, &QObject::deleteLater);
    _tcpThread->start();
    // Création du sender TCP
    _tcpSender = new CTcpSender(_tcpServer);
    _tcpSender->moveToThread(_tcpThread);
    // connecte
    connect(this, &CApp::sendTcpMessageRequested, _tcpSender, &CTcpSender::sendMessage);


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
