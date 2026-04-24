#pragma once
#include <QString>
#include <QUrl> // QUrl::toPercentEncoding

/**
 * @brief Stałe URL endpointów REST API GIOŚ.
 *
 * Dokumentacja: https://powietrze.gios.gov.pl/pjp/content/api
 */
namespace aqm::GIOSEndpoints {

/// Bazowy adres API.
inline const QString BASE = QStringLiteral("https://api.gios.gov.pl/pjp-api/v1/rest");

/// Endpoint: lista wszystkich stacji pomiarowych w Polsce.
inline const QString ALL_STATIONS = BASE + QStringLiteral("/station/findAll");

/**
 * @brief Endpoint: stanowiska pomiarowe danej stacji.
 * @param stationId  Identyfikator stacji.
 */
inline QString stationSensors(int stationId) {
    return BASE + QStringLiteral("/station/sensors/") + QString::number(stationId);
}

/**
 * @brief Endpoint: dane pomiarowe stanowiska.
 * @param sensorId  Identyfikator stanowiska.
 */
inline QString sensorData(int sensorId) {
    return BASE + QStringLiteral("/data/getData/") + QString::number(sensorId);
}

/**
 * @brief Endpoint: indeks jakości powietrza dla stacji.
 * @param stationId  Identyfikator stacji.
 */
inline QString airQualityIndex(int stationId) {
    return BASE + QStringLiteral("/aqindex/getIndex/") + QString::number(stationId);
}

/**
 * @brief Geokodowanie adresu tekstowego przez OpenStreetMap Nominatim.
 * @param query  Adres w formie tekstowej (np. "Polanka 3, Poznań").
 * @return       URL zapytania zwracający współrzędne w formacie JSON.
 */
inline QString nominatimSearch(const QString& query) {
    return QStringLiteral("https://nominatim.openstreetmap.org/search?q=")
           + QUrl::toPercentEncoding(query)
           + QStringLiteral("&format=json&limit=1");
}

} // namespace aqm::GIOSEndpoints