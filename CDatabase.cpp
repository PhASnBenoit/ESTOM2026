#include "CDatabase.h"

CDatabase::CDatabase(QObject *parent) : QThread(parent)
{
    // Connexion temporaire pour reset
    QSqlDatabase resetDb = QSqlDatabase::addDatabase("QMYSQL", "ResetConnection");
    resetDb.setHostName(MARIADB_HOST);
    resetDb.setPort(MARIADB_PORT);
    resetDb.setDatabaseName(MARIADB_DBNAME);
    resetDb.setUserName(MARIADB_NAME);
    resetDb.setPassword(MARIADB_PASSWORD);

    if (!resetDb.open()) {
        qDebug() << "Erreur : Impossible d'ouvrir la BDD pour le reset.";
        qDebug() << resetDb.lastError().text();
    } else {
        qDebug() << "Connexion temporaire pour reset réussie.";
        resetTables(resetDb);
    } // else

    // Supprimer la connexion temporaire pour éviter les conflits
    resetDb.close();
    resetDb = QSqlDatabase();
    QSqlDatabase::removeDatabase("ResetConnection");  // !!!!!!!!

    // La vraie connexion du thread sera ouverte dans run()
}

CDatabase::~CDatabase()
{
    requestInterruption();
    wait();
}

void CDatabase::resetTables(QSqlDatabase &db)
{
    QSqlQuery query(db);
    
    QStringList tables = {"BOM", "PAV"};
    for (const QString &table : tables) {
        //qDebug() << "Réinitialisation de la table :" << table;
        
        if (!query.exec("DELETE FROM " + table)) {
            qDebug() << "Erreur lors de la suppression des données de" << table << ":" << query.lastError().text();
        } // if delete
        if (!query.exec("ALTER TABLE " + table + " AUTO_INCREMENT = 1")) {
            qDebug() << "Erreur lors de la réinitialisation de l'ID de" << table << ":" << query.lastError().text();
        } // if alter
    } // for
    query.finish();
    qDebug() << "Tables BOM et PAV réinitialisées avec succès.";
} // method

void CDatabase::run()
{
    threadDb = QSqlDatabase::addDatabase("QMYSQL", "CDatabaseConnection");
    threadDb.setHostName(MARIADB_HOST);
    threadDb.setPort(MARIADB_PORT);
    threadDb.setDatabaseName(MARIADB_DBNAME);
    threadDb.setUserName(MARIADB_NAME);
    threadDb.setPassword(MARIADB_PASSWORD);
    if (!threadDb.open()) {
        qDebug() << "Erreur : Impossible d'ouvrir la base de données depuis le thread.";
        qDebug() << threadDb.lastError().text();
        return;
    } // if open

    T_CONFIG cfg;
    while (!isInterruptionRequested()) {
        QSqlQuery query(threadDb);
        if (!query.exec("SELECT * FROM Config")) {
            qDebug() << "Erreur SELECT Config :" << query.lastError().text();
            QThread::sleep(1);
            continue;
        } // if select
        if (!query.next()) {
            qDebug() << "La table Config est vide ou aucune ligne trouvée.";
            QThread::sleep(1);
            continue;
        } // if next
        cfg.options = query.value("Options").toString();
        cfg.ptsRecolte = query.value("PtsRecolte").toString();
        cfg.nbrPAV = query.value("NbrPAV").toString();
        cfg.status = query.value("Status").toString();
        cfg.luminosite = query.value("Luminosite").toString();
        // synchro WEB --> Serveur TCP
        emit configUpdated(cfg);
        QThread::sleep(1);  // Pause entre chaque lecture
    } // wh
    // en cas de sortie du thread
    threadDb.close();
    QSqlDatabase::removeDatabase("CDatabaseConnection");
}

bool CDatabase::insertDB(const QString &table, const QVariantList &values)
{
    if (values.isEmpty()) {
        qDebug() << "Erreur : aucune valeur fournie pour l'insertion.";
        return false;
    }

    // Construire la liste des placeholders "?" pour la requête préparée
    QStringList placeholders;
    for (int i = 0; i < values.size(); ++i) {
        placeholders.append("?");
    } // for

    QStringList columns;
    QStringList arg;
    columns.clear();
    if (table == "BOM") {
        switch(values.at(1).toInt()) {
        case 0: arg = {"IPAddr", "Couleur", "Status"}; break;  // BONJOUR
        case 1: arg = {"IPAddr", "Status"}; break;  // VIDAGE
        case 2: // FIN VIDAGE
        case 3: // ANN VIDAGE
            arg = {"IPAddr", "Status", "Remplissage"}; break;
        case 4: arg = {"IPAddr", "Status", "NbrCollision"}; break; // CHOC
        } // sw
    } else {   // "PAV"
        arg = {"IPAddr", "Status", "Couleur"};
    } // else
    columns.append(arg);

    if (columns.size() != values.size()) {
        qDebug() << "Erreur : Le nombre de valeurs ne correspond pas aux colonnes. Colum :" << columns.size() << "valeur:" << values.size();
        return false;
    } // if size

    // Construire la requête "ON DUPLICATE KEY UPDATE" 
    //permet de mettre à jour une ligne existante lorsqu'une contrainte d'unicité est violée lors d'un INSERT
    QStringList updateClauses;
    for (const QString &col : columns) {
        if (col != "IPAddr") { // Ne pas mettre à jour l'IPAddr car c'est la clé
            updateClauses.append(QString("%1 = VALUES(%1)").arg(col));
        } // if
    } // for col

    QString queryString = QString(
        "INSERT INTO %1 (%2) VALUES (%3) "
        "ON DUPLICATE KEY UPDATE %4"
    ).arg(table)
     .arg(columns.join(", "))
     .arg(placeholders.join(", "))
     .arg(updateClauses.join(", "));
    
    QString ip;
    ip = values[0].toString();
/*
    if (table == "PAV" && values.size() > 3) {
    } else if (table == "BOM" && values.size() > 5) {
        ip = values[5].toString();
    } else {
        ip = "Unknown";
        qDebug() << "CDatabase::insertDB TABLE inconnue";
    } // else
*/
    qDebug() << "Commande suite ip :" << ip << " " << queryString;

    QSqlQuery query(threadDb);
    query.prepare(queryString);
    for (const QVariant &value : values) {
        query.addBindValue(value);
    } // for
    if (!query.exec()) {
        qDebug() << "Erreur lors de l'insertion :" << query.lastError().text();
        return false;
    } // if
    return true;
}


QStringList CDatabase::getAllIPs()
{
    QStringList ipList;
    QSqlQuery query(threadDb);

    if (!query.exec("SELECT IPAddr FROM BOM UNION SELECT IPAddr FROM PAV")) {
        qDebug() << "Erreur lors de la récupération des IPs:" << query.lastError().text();
        return ipList;
    } // if exec

    while (query.next()) {
        ipList.append(query.value(0).toString());
    } // wh

    return ipList;
}

T_SEND CDatabase::getDataToSend()
{
  T_SEND toSend;
/*  QSqlQuery query(threadDb);
  QString req;
  req = "SELECT BOM.Status, PAV.Status, NbrCollision, Remplissage, Luminosite ";
  req+= "FROM Config, BOM, PAV ";
  req+= "WHERE IPAddr = ";

  if (!query.exec(req)) {
      qDebug() << "Erreur lors de la récupération des IPs:" << query.lastError().text();
      return ipList;
  } // if exec

  query.next();
  toSend.etatB = query.value(0).toString();
  toSend.etatP = query.value(1).toString();
  toSend.etatB = query.value(2).toString();
  toSend.etatB = query.value(3).toString();
  toSend.etatB = query.value(4).toString();
  */
  return toSend;

}
