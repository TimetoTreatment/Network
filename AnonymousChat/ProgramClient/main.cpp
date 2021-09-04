#include "ProgramClient.h"
#include <QtWidgets/QApplication>
#include <thread>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ProgramClient w;
    w.show();
    return a.exec();
}
