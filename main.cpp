#include <QtGui/QApplication>
#include "piqs.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Piqs w;
    w.show();

    return a.exec();
}
