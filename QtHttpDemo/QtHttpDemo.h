#pragma once

#include "ui_QtHttpDemo.h"

#include <QMainWindow>

class QNetworkAccessManager;

class QtHttpDemo : public QMainWindow
{
    // moc 根据这个宏生成窗口类的 Qt 元对象代码。
    Q_OBJECT

public:
    explicit QtHttpDemo(QWidget *parent = nullptr);

private:
    // 两个按钮共用请求流程。isPost 决定使用 POST 请求体还是直接发送 GET。
    void sendRequest(bool isPost);

    // uic 从 QtHttpDemo.ui 生成这个类型，setupUi 后可用成员访问各个控件。
    Ui::QtHttpDemoClass ui;
    // 窗口是管理器的 QObject 父对象，同一个管理器处理窗口发出的所有请求。
    QNetworkAccessManager *networkManager;
};
