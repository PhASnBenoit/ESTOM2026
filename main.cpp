#include "CApp.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CApp w;
    //w.show();

    return a.exec();
}

