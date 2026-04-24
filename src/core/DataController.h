#pragma once
#include <QObject>
#include <QString>
#include <memory>
#include <vector>
#include "model/Station.h"
#include "model/Sensor.h"
#include "model/Measurement.h"
#include "model/AirQualityIndex.h"
#include "storage/IDatabase.h"

namespace aqm {

/**
 * @brief Centralny kontroler danych aplikacji.
 *
 * Koordynuje pobieranie danych z API GIOŚ (w wątkach roboczych),
 * zapisywanie ich do lokalnej bazy danych oraz fallback na dane
 * historyczne przy braku połączenia sieciowego.
 *
 * Wszystkie operacje sieciowe wykonywane są asynchronicznie
 * (QThread::create).  Wyniki przekazywane są przez sygnały Qt.
 *
 * Implementuje wzorzec Singleton przez statyczną metodę @ref instance.
 */
class DataController : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Zwraca jedyną instancję kontrolera (Singleton).
     * @param parent  Używany tylko przy pierwszym wywołaniu.
     */
    static DataController* instance(QObject* parent = nullptr);

    /**
     * @brief Ustawia implementację bazy danych (Strategy).
     * @param db  Wskaźnik do obiektu implementującego IDatabase.
     */
    void setDatabase(std::unique_ptr<IDatabase> db);

    // ── Operacje asynchroniczne ───────────────────────────────────────────────

    /** @brief Pobiera listę wszystkich stacji (API → DB fallback). */
    void fetchAllStations();

    /**
     * @brief Pobiera stanowiska pomiarowe stacji.
     * @param stationId  Identyfikator stacji.
     */
    void fetchSensors(int stationId);

    /**
     * @brief Pobiera dane pomiarowe stanowiska.
     * @param sensorId  Identyfikator stanowiska.
     * @param save      Jeśli true – zapisuje wynik w lokalnej bazie.
     */
    void fetchMeasurements(int sensorId, bool save = false);

    /**
     * @brief Pobiera indeks jakości powietrza dla stacji.
     * @param stationId  Identyfikator stacji.
     */
    void fetchAirQualityIndex(int stationId);

    /**
     * @brief Geokoduje adres tekstowy i wyszukuje stacje w promieniu.
     * @param address   Opis słowny lokalizacji (np. "Polanka 3, Poznań").
     * @param radiusKm  Promień wyszukiwania w kilometrach.
     * @param allStations  Cała lista stacji (filtrowana lokalnie).
     */
    void fetchStationsNearAddress(const QString& address, double radiusKm,
                                  const std::vector<Station>& allStations);

    /**
     * @brief Wczytuje pomiary z lokalnej bazy danych (synchronicznie).
     * @param sensorId  Identyfikator stanowiska.
     */
    void loadMeasurementsFromDb(int sensorId);

    /**
     * @brief Zapisuje serię pomiarową do lokalnej bazy (synchronicznie).
     * @param m  Seria do zapisu.
     */
    void saveMeasurements(const Measurement& m);

signals:
    /** @brief Emitowany po załadowaniu listy stacji. */
    void stationsReady(std::vector<aqm::Station> stations);
    /** @brief Emitowany po załadowaniu stanowisk. */
    void sensorsReady(std::vector<aqm::Sensor> sensors);
    /** @brief Emitowany po załadowaniu danych pomiarowych. */
    void measurementsReady(aqm::Measurement measurement);
    /** @brief Emitowany po załadowaniu indeksu jakości. */
    void airQualityIndexReady(aqm::AirQualityIndex index);
    /** @brief Emitowany po filtrowaniu po promieniu. */
    void nearbyStationsReady(std::vector<aqm::Station> stations);
    /** @brief Komunikat błędu (np. brak sieci). */
    void errorOccurred(QString message);
    /** @brief Komunikat statusu (np. "Ładowanie..."). */
    void statusMessage(QString message);

private:
    explicit DataController(QObject* parent = nullptr);
    static DataController* s_instance;

    std::unique_ptr<IDatabase> m_db; ///< Aktywna baza danych

    /** @brief Uruchamia funkcję w wątku roboczym i obsługuje wyjątki. */
    template<typename Func>
    void runAsync(Func&& func);
};

} // namespace aqm
