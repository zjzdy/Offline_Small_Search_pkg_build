#include "opkg_build.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    opkg_build w;
    w.show();

    return a.exec();
}
