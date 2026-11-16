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
    // 先根据 .ui 创建控件，再填入本地回显服务的 URL 和中文 JSON 示例。
    ui.setupUi(this);
	// 编译时成为静态数据，运行时引用
    ui.urlEdit->setText(QStringLiteral("http://127.0.0.1:8080/echo?name=Qt"));
    ui.bodyEdit->setPlainText(QString::fromUtf8(u8"{\"message\":\"你好，Qt\"}"));

    // 按钮的 clicked 信号决定请求方法，输入值在每次点击时从控件读取。
    connect(ui.getButton, &QPushButton::clicked, this, [this] { sendRequest(false); });
    connect(ui.postButton, &QPushButton::clicked, this, [this] { sendRequest(true); });
}

void QtHttpDemo::sendRequest(bool isPost)
{
    // 开始处理本次请求时清除旧正文，响应区只显示这一次的结果。
    ui.responseEdit->clear();
    // QUrl 负责解释 URL 的组成部分。这里用严格模式，并要求 HTTP 协议和主机地址。
    const QUrl url(ui.urlEdit->text().trimmed(), QUrl::StrictMode);
    if (!url.isValid() || url.scheme() != QStringLiteral("http") || url.host().isEmpty()) {
        ui.statusLabel->setText(QString::fromUtf8(u8"请输入完整的 http:// URL"));
        return;
    }

    // 两个按钮共用状态标签和响应区。请求完成前禁用发送，保证界面一次处理一个请求。
    ui.getButton->setEnabled(false);
    ui.postButton->setEnabled(false);
    ui.statusLabel->setText(QString::fromUtf8(u8"请求中..."));

    // QNetworkRequest 保存 URL 和请求头。POST 的正文作为单独的字节数组传给管理器。
    QNetworkRequest request(url);
    QNetworkReply *reply;
    if (isPost) {
        // 输入框返回 QString。toUtf8() 将输入原文编码为 QByteArray，中文也按 UTF-8 发送。
        // Content-Type 声明正文的媒体类型和字符编码。
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QStringLiteral("application/json; charset=utf-8"));
        reply = networkManager->post(request, ui.bodyEdit->toPlainText().toUtf8());
    } else {
        // GET 参数已在 URL 的查询部分，例如 ?name=Qt。此调用只传请求描述。
        reply = networkManager->get(request);
    }

    // get() 和 post() 先返回代表本次请求的 QNetworkReply，实际响应由事件循环异步处理。
    // 定时回调以 reply 为上下文，回复对象销毁后回调自动失效。
    // 超时时先给回复打标记再 abort()，完成回调就能显示具体的超时提示。
    QTimer::singleShot(10000, Qt::PreciseTimer, reply, [reply] {
        if (!reply->isFinished()) {
            reply->setProperty("demoTimedOut", true);
            reply->abort();
        }
    });

    // 成功、HTTP 错误和连接失败都在 finished 回调中收尾。
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        // Qt Network 已经解析 HTTP 响应，状态码作为回复属性提供。
        const QVariant status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        // readAll() 读出尚未读取的正文字节。解码得到 QString 后，直接显示原始文本。
        // 即使状态为 404，只要正文可读，也会显示服务返回的错误信息。
        if (reply->isReadable()) {
            ui.responseEdit->setPlainText(QString::fromUtf8(reply->readAll()));
        }

        // 有 HTTP 响应时先显示状态码。连接被拒绝等错误通常由 Qt 提供错误描述。
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

        // 所有已发起的请求都在这里恢复发送按钮，最后把回复对象交给事件循环清理。
        ui.statusLabel->setText(message);
        ui.getButton->setEnabled(true);
        ui.postButton->setEnabled(true);
        reply->deleteLater();
    });
}
