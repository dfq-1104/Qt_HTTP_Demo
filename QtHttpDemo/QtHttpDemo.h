#pragma once

#include "ui_QtHttpDemo.h"

#include <QMainWindow>

class QNetworkAccessManager;

class QtHttpDemo : public QMainWindow
{
    Q_OBJECT

public:
    explicit QtHttpDemo(QWidget *parent = nullptr);

private:
    void sendRequest(bool isPost);

    Ui::QtHttpDemoClass ui;
    QNetworkAccessManager *networkManager;
};
