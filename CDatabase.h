#ifndef CDATABASE_H
#define CDATABASE_H

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
    QString whoAmI(const QString &ip);

protected:
    void run() override;  // Méthode exécutée en boucle dans le thread

private:
    QSqlDatabase threadDb;
    void resetTables(QSqlDatabase &db);
signals:
    void configUpdated(T_CONFIG cfg);
    void ipListUpdated(QStringList ipList);
};

#endif // CDBREADER_H
