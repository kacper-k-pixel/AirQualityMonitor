#pragma once
#include <QObject>
#include <QByteArray>
#include <QString>
#include <stdexcept>

class QNetworkAccessManager;

namespace aqm {

/**
 * @brief Wyjątek rzucany przy błędach sieciowych.
 *
 * Opakowuje kody błędów Qt Network w standardowy C++ exception.
 */
class NetworkException : public std::runtime_error {
public:
    /**
     * @brief Konstruktor.
     * @param msg  Opis błędu.
     */
    explicit NetworkException(const QString& msg)
        : std::runtime_error(msg.toStdString()) {}
};

/**
 * @brief Klient HTTP wykonujący synchroniczne żądania GET.
 *
 * Wewnętrznie używa QNetworkAccessManager z lokalną QEventLoop,
 * dzięki czemu blokuje wywołujący wątek do czasu odebrania odpowiedzi.
 *
 * @warning Nie używać w głównym wątku GUI – przeznaczony do uruchamiania
 *          z wątków roboczych (np. QThread::create).
 */
class RestClient : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Konstruktor – tworzy własny QNetworkAccessManager.
     * @param parent  Obiekt nadrzędny Qt.
     */
    explicit RestClient(QObject* parent = nullptr);

    /**
     * @brief Wykonuje żądanie HTTP GET i zwraca treść odpowiedzi.
     *
     * Blokuje wywołujący wątek maksymalnie przez @p timeoutMs milisekund.
     *
     * @param url        Adres URL zasobu.
     * @param timeoutMs  Limit czasu w milisekundach (domyślnie 15 000).
     * @return           Surowe bajty odpowiedzi serwera.
     * @throws NetworkException  Przy błędzie sieci lub przekroczeniu limitu.
     */
    QByteArray get(const QString& url, int timeoutMs = 15000);

private:
    QNetworkAccessManager* m_manager; ///< Menedżer połączeń HTTP
};

} // namespace aqm
