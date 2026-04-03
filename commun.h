#ifndef COMMUN_H
#define COMMUN_H

#include <QString>

// éléments de la table config à communiquer
typedef struct s_config {
    QString options,
            ptsRecolte,
            nbrPAV,
            status,
            luminosite;
} T_CONFIG;

// éléments de la communication venant d'un périphérique
typedef struct s_info {
    QString status,
            type,
            couleur,
            nbCollisions,
            ipPAV,
            leds;
} T_INFOS;

// éléments utiles pour émettre vers les périphériques
typedef struct s_send {
    QString pb,  // "P" pour PAV   "B" pour BOM
            etatB,  // état du périphérique BOM
            etatP,  // état du périph PAV
            nbChocs,
            leds,  // progression du véhicule
            luminosite; // O faible, 1 moyen, 2 fort
} T_SEND;

#define TCP_PORT         5005
#define MARIADB_HOST     "localhost"
#define MARIADB_PORT     3306
#define MARIADB_DBNAME   "ESTOM"
#define MARIADB_NAME     "SuperViseur"
#define MARIADB_PASSWORD "SuperViseur-estom_2026"

#endif // COMMUN_H
/*
 * A FAIRE
 *
 * CDatabase::getDataToSend à terminer
 */
