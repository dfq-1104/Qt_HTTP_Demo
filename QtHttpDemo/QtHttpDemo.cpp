#include "QtHttpDemo.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QTimer>
#include <QUrl>
#include <QVariant>

QtHttpDemo::QtHttpDemo(QWidget *parent)
    : QMainWindow(parent), networkManager(new QNetworkAccessManager(this))
{
    ui.setupUi(this);
    ui.urlEdit->setText(QStringLiteral("http://127.0.0.1:8080/echo?name=Qt"));
    ui.bodyEdit->setPlainText(QString::fromUtf8(u8"{\"message\":\"你好，Qt\"}"));

    connect(ui.getButton, &QPushButton::clicked, this, [this] { sendRequest(false); });
    connect(ui.postButton, &QPushButton::clicked, this, [this] { sendRequest(true); });
}

void QtHttpDemo::sendRequest(bool isPost)
{
    ui.responseEdit->clear();
    const QUrl url(ui.urlEdit->text().trimmed(), QUrl::StrictMode);
    if (!url.isValid() || url.scheme() != QStringLiteral("http") || url.host().isEmpty()) {
        ui.statusLabel->setText(QString::fromUtf8(u8"请输入完整的 http:// URL"));
        return;
    }

    ui.getButton->setEnabled(false);
    ui.postButton->setEnabled(false);
    ui.statusLabel->setText(QString::fromUtf8(u8"请求中..."));

    QNetworkRequest request(url);
    QNetworkReply *reply;
    if (isPost) {
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QStringLiteral("application/json; charset=utf-8"));
        reply = networkManager->post(request, ui.bodyEdit->toPlainText().toUtf8());
    } else {
        reply = networkManager->get(request);
    }

    QTimer::singleShot(10000, Qt::PreciseTimer, reply, [reply] {
        if (!reply->isFinished()) {
            reply->setProperty("demoTimedOut", true);
            reply->abort();
        }
    });

    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        const QVariant status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (reply->isReadable()) {
            ui.responseEdit->setPlainText(QString::fromUtf8(reply->readAll()));
        }

        QString message;
        if (status.isValid()) {
            message = QStringLiteral("HTTP %1").arg(status.toInt());
        }
        if (reply->property("demoTimedOut").toBool()) {
            message = QString::fromUtf8(u8"请求超时 (10 秒)");
        } else if (reply->error() != QNetworkReply::NoError) {
            if (!message.isEmpty()) {
                message += QStringLiteral(" - ");
            }
            message += reply->errorString();
        }

        ui.statusLabel->setText(message);
        ui.getButton->setEnabled(true);
        ui.postButton->setEnabled(true);
        reply->deleteLater();
    });
}
