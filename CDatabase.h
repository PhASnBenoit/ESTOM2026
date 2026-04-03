// CDatabase.h
#pragma once

#include <QThread>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

#include "commun.h"

class CDatabase : public QThread
{
    Q_OBJECT

public:
    explicit CDatabase(QObject *parent = nullptr);
    ~CDatabase();
    bool insertDB(const QString &table, const QVariantList &values);
    bool checkPAV(const QString &ipPAV);
    QStringList getAllIPs();
    T_SEND getDatasToSend(QString ip, int ordre, T_SEND toSend);

protected:
    void run() override;  // Méthode exécutée en boucle dans le thread

private:
    QSqlDatabase threadDb;
    void resetTables(QSqlDatabase &db);

signals:
    void sig_configUpdated(T_CONFIG cfg);
    //void sig_ipListUpdated(QStringList ipList);
};
