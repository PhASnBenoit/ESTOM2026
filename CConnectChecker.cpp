#include "CConnectChecker.h"

CConnectChecker::CConnectChecker(QObject *parent)
    : QThread(parent), shouldStop(false) {}

CConnectChecker::~CConnectChecker() {
    stop();
    wait();
}

void CConnectChecker::stop() {
    shouldStop = true;
}

QSqlDatabase CConnectChecker::getDatabaseConnection()
{
    QString connectionName = QString("checkerConnection_%1").arg(reinterpret_cast<quintptr>(QThread::currentThread()));

    if (QSqlDatabase::contains(connectionName)) {
        return QSqlDatabase::database(connectionName);
    } // if contains

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", connectionName);
    db.setHostName(MARIADB_HOST);
    db.setPort(MARIADB_PORT);
    db.setDatabaseName(MARIADB_DBNAME);
    db.setUserName(MARIADB_NAME);
    db.setPassword(MARIADB_PASSWORD);
    if (!db.open()) {
        qDebug() << "Erreur : Impossible d'ouvrir la base de données.";
        qDebug() << db.lastError().text();
    } // if open
    return db;
}

void CConnectChecker::run()
{
    while (!shouldStop) {
        QStringList ipList = getAllIPs();  // Met à jour la liste à chaque cycle
        for (const QString &ip : ipList) {
            if (shouldStop) break;  // Vérifie l'arrêt en cours d'exécution
            bool isConnected = pingIP(ip);
            updateConnectStatus(ip, isConnected ? 1 : 0);
        } // for ipList
        for (int i = 0; i < 50 && !shouldStop; ++i) {
            QThread::msleep(100);  // Pause plus réactive (5s au total)
        } // for
    } // wh
    // Nettoyage de la connexion après utilisation
    QSqlDatabase::removeDatabase(QSqlDatabase::database().connectionName());
}

bool CConnectChecker::pingIP(const QString &ip) {
    QProcess process;
    QString command = QString("ping -c 1 -W 1 %1").arg(ip);  // Teste une seule fois avec un timeout de 1s
    process.start(command);
    process.waitForFinished(2000);  // Timeout général de 2s
    return process.exitCode() == 0; // Retourne true si l'IP répond au ping
}

int CConnectChecker::getConnectStatus(const QString &ip)
{
    QSqlDatabase db = getDatabaseConnection();
    QSqlQuery query(db);
    query.prepare("SELECT Connected FROM BOM WHERE IPAddr = :ip UNION SELECT Connected FROM PAV WHERE IPAddr = :ip");
    query.bindValue(":ip", ip);
    if (!query.exec()) {
        qDebug() << "Erreur lors de la récupération du statut de connexion pour" << ip << ":" << query.lastError().text();
        return -1;
    } // if exec
    if (query.next()) {
        return query.value(0).toInt();
    } // if next

    return -1;
}

QStringList CConnectChecker::getAllIPs()
{
    QStringList ipList;
    QSqlDatabase db = getDatabaseConnection();
    QSqlQuery query(db);
    if (!query.exec("SELECT IPAddr FROM BOM UNION SELECT IPAddr FROM PAV")) {
        qDebug() << "Erreur lors de la récupération des IPs:" << query.lastError().text();
        return ipList;
    } // if exec
    while (query.next()) {
        ipList.append(query.value(0).toString());
    } // wh next
    //qDebug() << "Liste des IPs : " << ipList;
    return ipList;
}

void CConnectChecker::updateConnectStatus(const QString &ip, int status)
{
    QSqlDatabase db = getDatabaseConnection();
    QSqlQuery query(db);

    query.prepare("UPDATE BOM SET Connected = :status WHERE IPAddr = :ip");
    query.bindValue(":status", status);
    query.bindValue(":ip", ip);
    if (!query.exec()) {
        qDebug() << "Erreur lors de la mise à jour de BOM:" << query.lastError().text();
    } // if exec
    query.prepare("UPDATE PAV SET Connected = :status WHERE IPAddr = :ip");
    query.bindValue(":status", status);
    query.bindValue(":ip", ip);
    if (!query.exec()) {
        qDebug() << "Erreur lors de la mise à jour de PAV:" << query.lastError().text();
    } // if exec
}
