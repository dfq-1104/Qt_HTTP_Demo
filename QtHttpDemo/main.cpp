#include "QtHttpDemo.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QtHttpDemo window;
    window.show();
    return app.exec();
}
