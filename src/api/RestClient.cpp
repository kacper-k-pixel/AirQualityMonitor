#include "RestClient.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QUrl>

namespace aqm {

RestClient::RestClient(QObject* parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{}

QByteArray RestClient::get(const QString& url, int timeoutMs) {
    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("AirQualityMonitor/1.0"));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply* reply = m_manager->get(request);

    // Blokujemy wątek roboczy lokalną pętlą zdarzeń do czasu
    // odebrania odpowiedzi lub upłynięcia limitu timeoutMs.
    QEventLoop loop;
    QTimer     timer;
    timer.setSingleShot(true);
    timer.setInterval(timeoutMs);

    QObject::connect(reply,  &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout,         &loop, &QEventLoop::quit);
    timer.start();
    loop.exec();

    if (!timer.isActive()) {
        reply->abort();
        reply->deleteLater();
        throw NetworkException(
            QStringLiteral("Przekroczono limit czasu (%1 ms) dla: %2")
                .arg(timeoutMs).arg(url));
    }
    timer.stop();

    if (reply->error() != QNetworkReply::NoError) {
        const QString msg = reply->errorString();
        reply->deleteLater();
        throw NetworkException(
            QStringLiteral("Błąd sieci: %1 [%2]").arg(msg, url));
    }

    const QByteArray data = reply->readAll();
    reply->deleteLater();
    return data;
}

} // namespace aqm