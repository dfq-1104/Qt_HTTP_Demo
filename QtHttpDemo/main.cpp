#include "QtHttpDemo.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QtHttpDemo window;
    window.show();
    // 进入 Qt 事件循环。按钮点击、网络回复和超时回调都由事件循环派发。
    return app.exec();
}
