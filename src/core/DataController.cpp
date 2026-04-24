#include "DataController.h"
#include "api/RestClient.h"
#include "api/JsonParser.h"
#include "api/GIOSEndpoints.h"
#include <nlohmann/json.hpp>
#include <QThread>
#include <QMetaObject>
#include <cmath>

namespace aqm {

DataController* DataController::s_instance = nullptr;

// ── Singleton ─────────────────────────────────────────────────────────────────

DataController* DataController::instance(QObject* parent) {
    if (!s_instance)
        s_instance = new DataController(parent);
    return s_instance;
}

DataController::DataController(QObject* parent) : QObject(parent) {}

void DataController::setDatabase(std::unique_ptr<IDatabase> db) {
    m_db = std::move(db);
}

// ── Async helper ──────────────────────────────────────────────────────────────

template<typename Func>
void DataController::runAsync(Func&& func) {
    auto* thread = QThread::create(std::forward<Func>(func));
    thread->setParent(this);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

// ── Fetch All Stations ────────────────────────────────────────────────────────

void DataController::fetchAllStations() {
    emit statusMessage(QStringLiteral("Pobieranie stacji z API GIOŚ..."));
    runAsync([this]() {
        try {
            RestClient client;
            std::vector<Station> allStations;
            int page = 0;

            while (true) {
                const QString url = GIOSEndpoints::ALL_STATIONS
                                    + QStringLiteral("?page=%1&size=500").arg(page);
                const auto raw = client.get(url);

                nlohmann::json j;
                try { j = nlohmann::json::parse(raw.toStdString()); }
                catch (...) { break; }

                auto batch = JsonParser::parseStations(raw);
                if (batch.empty()) break;

                allStations.insert(allStations.end(),
                                   batch.begin(), batch.end());

                bool hasMore = false;
                if (j.contains("totalPages")) {
                    hasMore = (page + 1) < j.at("totalPages").get<int>();
                } else if (j.contains("links")) {
                    hasMore = j.at("links").contains("next");
                }

                if (!hasMore) break;
                page++;
            }

            if (m_db) {
                try { m_db->saveStations(allStations); } catch (...) {}
            }

            QMetaObject::invokeMethod(this,
                [this, s = std::move(allStations)]() mutable {
                    emit statusMessage(
                        QStringLiteral("Załadowano %1 stacji.").arg(s.size()));
                    emit stationsReady(std::move(s));
                }, Qt::QueuedConnection);

        } catch (const std::exception& e) {
            const QString err = QString::fromStdString(e.what());
            if (m_db) {
                try {
                    auto stations = m_db->loadStations();
                    QMetaObject::invokeMethod(this,
                        [this, s = std::move(stations)]() mutable {
                            emit statusMessage(
                                QStringLiteral("Dane z bazy lokalnej (%1 stacji).")
                                    .arg(s.size()));
                            emit stationsReady(std::move(s));
                        }, Qt::QueuedConnection);
                    return;
                } catch (...) {}
            }
            QMetaObject::invokeMethod(this, [this, err]() {
                emit errorOccurred(
                    QStringLiteral("Nie można pobrać stacji: ") + err);
            }, Qt::QueuedConnection);
        }
    });
}

// ── Fetch Sensors ─────────────────────────────────────────────────────────────

void DataController::fetchSensors(int stationId) {
    emit statusMessage(
        QStringLiteral("Pobieranie stanowisk stacji %1...").arg(stationId));
    runAsync([this, stationId]() {
        try {
            RestClient client;
            std::vector<Sensor> allSensors;
            int page = 0;

            while (true) {
                const QString url = GIOSEndpoints::stationSensors(stationId)
                                    + QStringLiteral("?page=%1&size=100").arg(page);
                const auto raw = client.get(url);

                auto batch = JsonParser::parseSensors(raw);
                if (batch.empty()) break;

                allSensors.insert(allSensors.end(),
                                  batch.begin(), batch.end());

                nlohmann::json j;
                try { j = nlohmann::json::parse(raw.toStdString()); }
                catch (...) { break; }

                bool hasMore = false;
                if (j.contains("totalPages"))
                    hasMore = (page + 1) < j.at("totalPages").get<int>();
                if (!hasMore) break;
                page++;
            }

            if (m_db) {
                try { m_db->saveSensors(stationId, allSensors); } catch (...) {}
            }

            QMetaObject::invokeMethod(this,
                [this, s = std::move(allSensors)]() mutable {
                    emit sensorsReady(std::move(s));
                }, Qt::QueuedConnection);

        } catch (const std::exception& e) {
            const QString err = QString::fromStdString(e.what());
            if (m_db) {
                try {
                    auto sensors = m_db->loadSensors(stationId);
                    QMetaObject::invokeMethod(this,
                        [this, s = std::move(sensors)]() mutable {
                            emit statusMessage(
                                QStringLiteral("Stanowiska z bazy lokalnej."));
                            emit sensorsReady(std::move(s));
                        }, Qt::QueuedConnection);
                    return;
                } catch (...) {}
            }
            QMetaObject::invokeMethod(this, [this, err]() {
                emit errorOccurred(
                    QStringLiteral("Nie można pobrać stanowisk: ") + err);
            }, Qt::QueuedConnection);
        }
    });
}

// ── Fetch Measurements ────────────────────────────────────────────────────────

void DataController::fetchMeasurements(int sensorId, bool save) {
    emit statusMessage(
        QStringLiteral("Pobieranie pomiarów stanowiska %1...").arg(sensorId));
    runAsync([this, sensorId, save]() {
        try {
            RestClient client;
            Measurement combined;
            combined.sensorId = sensorId;
            int page = 0;

            while (true) {
                const QString url = GIOSEndpoints::sensorData(sensorId)
                                    + QStringLiteral("?page=%1&size=100").arg(page);
                const auto raw = client.get(url);

                nlohmann::json j;
                try { j = nlohmann::json::parse(raw.toStdString()); }
                catch (...) { break; }

                auto batch = JsonParser::parseMeasurements(raw, sensorId);

                if (combined.key.isEmpty())
                    combined.key = batch.key;

                combined.values.insert(combined.values.end(),
                                       batch.values.begin(),
                                       batch.values.end());

                bool hasMore = false;
                if (j.contains("totalPages"))
                    hasMore = (page + 1) < j.at("totalPages").get<int>();
                if (!hasMore) break;
                page++;
            }

            if (save && m_db) {
                try { m_db->saveMeasurements(combined); } catch (...) {}
            }

            QMetaObject::invokeMethod(this,
                [this, m = std::move(combined)]() mutable {
                    emit statusMessage(
                        QStringLiteral("Załadowano %1 odczytów.")
                            .arg(m.values.size()));
                    emit measurementsReady(std::move(m));
                }, Qt::QueuedConnection);

        } catch (const std::exception& e) {
            const QString err = QString::fromStdString(e.what());

            // Kod 400 = stanowisko manualne, dane niedostępne na bieżąco
            if (err.contains(QStringLiteral("400"))) {
                QMetaObject::invokeMethod(this, [this]() {
                    emit statusMessage(QStringLiteral(
                        "Stanowisko manualne – dane niedostępne na bieżąco."));
                }, Qt::QueuedConnection);
                return;
            }

            if (m_db && m_db->hasMeasurements(sensorId)) {
                try {
                    auto m = m_db->loadMeasurements(sensorId);
                    QMetaObject::invokeMethod(this,
                        [this, m2 = std::move(m)]() mutable {
                            emit statusMessage(
                                QStringLiteral("Pomiary z bazy lokalnej (%1 odczytów).")
                                    .arg(m2.values.size()));
                            emit measurementsReady(std::move(m2));
                        }, Qt::QueuedConnection);
                    return;
                } catch (...) {}
            }
            QMetaObject::invokeMethod(this, [this, err]() {
                emit errorOccurred(
                    QStringLiteral("Nie można pobrać pomiarów: ") + err);
            }, Qt::QueuedConnection);
        }
    });
}

// ── Fetch AQI ─────────────────────────────────────────────────────────────────

void DataController::fetchAirQualityIndex(int stationId) {
    runAsync([this, stationId]() {
        try {
            RestClient client;
            const auto raw = client.get(GIOSEndpoints::airQualityIndex(stationId));
            auto idx = JsonParser::parseAirQualityIndex(raw, stationId);
            QMetaObject::invokeMethod(this,
                [this, idx2 = std::move(idx)]() mutable {
                    emit airQualityIndexReady(std::move(idx2));
                }, Qt::QueuedConnection);
        } catch (const std::exception& e) {
            const QString err = QString::fromStdString(e.what());
            QMetaObject::invokeMethod(this, [this, err]() {
                emit errorOccurred(
                    QStringLiteral("Nie można pobrać indeksu: ") + err);
            }, Qt::QueuedConnection);
        }
    });
}

// ── Nearby Stations ───────────────────────────────────────────────────────────

static double haversineKm(double lat1, double lon1, double lat2, double lon2) {
    constexpr double R = 6371.0; // promień Ziemi w km
    const double dLat = (lat2 - lat1) * M_PI / 180.0;
    const double dLon = (lon2 - lon1) * M_PI / 180.0;
    const double a = std::sin(dLat/2) * std::sin(dLat/2)
                   + std::cos(lat1 * M_PI / 180.0) * std::cos(lat2 * M_PI / 180.0)
                   * std::sin(dLon/2) * std::sin(dLon/2);
    return R * 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
}

void DataController::fetchStationsNearAddress(const QString& address,
                                               double radiusKm,
                                               const std::vector<Station>& allStations) {
    emit statusMessage(
        QStringLiteral("Geokodowanie adresu: %1...").arg(address));
    auto stations = allStations; // kopia by uniknąć data race
    runAsync([this, address, radiusKm, stations = std::move(stations)]() mutable {
        try {
            RestClient client;
            const auto raw = client.get(GIOSEndpoints::nominatimSearch(address));
            const auto [lat, lon] = JsonParser::parseNominatimCoords(raw);

            std::vector<Station> nearby;
            for (const auto& s : stations)
                if (haversineKm(lat, lon, s.gegrLat, s.gegrLon) <= radiusKm)
                    nearby.push_back(s);

            QMetaObject::invokeMethod(this,
                [this, n = std::move(nearby), radiusKm]() mutable {
                    emit statusMessage(
                        QStringLiteral("Znaleziono %1 stacji w promieniu %2 km.")
                            .arg(n.size()).arg(radiusKm));
                    emit nearbyStationsReady(std::move(n));
                }, Qt::QueuedConnection);
        } catch (const std::exception& e) {
            const QString err = QString::fromStdString(e.what());
            QMetaObject::invokeMethod(this, [this, err]() {
                emit errorOccurred(
                    QStringLiteral("Geokodowanie nie powiodło się: ") + err);
            }, Qt::QueuedConnection);
        }
    });
}

// ── Synchronous DB ops ────────────────────────────────────────────────────────

void DataController::loadMeasurementsFromDb(int sensorId) {
    if (!m_db) { emit errorOccurred(QStringLiteral("Brak bazy danych.")); return; }
    try {
        auto m = m_db->loadMeasurements(sensorId);
        emit measurementsReady(std::move(m));
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromStdString(e.what()));
    }
}

void DataController::saveMeasurements(const Measurement& m) {
    if (!m_db) { emit errorOccurred(QStringLiteral("Brak bazy danych.")); return; }
    try {
        m_db->saveMeasurements(m);
        emit statusMessage(QStringLiteral("Pomiary zapisane w bazie lokalnej."));
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromStdString(e.what()));
    }
}

} // namespace aqm