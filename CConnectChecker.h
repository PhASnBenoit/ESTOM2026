#ifndef CCONNECTCHECKER_H
#define CCONNECTCHECKER_H

#include <QObject>
#include <QThread>
#include <QStringList>
#include <QProcess>
#include <QtConcurrent/QtConcurrent>
#include <QDebug>
#include <QSqlError>

#include "commun.h"
#include "CDatabase.h"

class CConnectChecker : public QThread
{
    Q_OBJECT
public:
    explicit CConnectChecker(QObject *parent = nullptr);
    ~CConnectChecker();


    void stop();
protected:
    void run() override;

private:
    bool shouldStop;

    QSqlDatabase getDatabaseConnection();
    bool pingIP(const QString &ip);
    int getConnectStatus(const QString &ip);

    QStringList getAllIPs();
    void updateConnectStatus(const QString &ip, int status);
    
};

#endif // CCONNECTCHECKER_H
