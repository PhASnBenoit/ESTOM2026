#ifndef COMMUN_H
#define COMMUN_H

#include <QString>

typedef struct s_config {
    QString options,
            ptsRecolte,
            nbrPAV,
            status,
            luminosite;
} T_CONFIG;

typedef struct s_info {
    QString status,
            type,
            couleur,
            nbCollisions,
            ipPAV,
            leds;
} T_INFOS;

#endif // COMMUN_H
