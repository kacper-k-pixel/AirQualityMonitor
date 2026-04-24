#pragma once
#include "IDatabase.h"
#include <QString>

namespace aqm {

/**
 * @brief Implementacja lokalnej bazy danych w formacie JSON.
 *
 * Dane przechowywane są w plikach JSON w katalogu wskazanym
 * w konstruktorze:
 * - @c stations.json – lista wszystkich stacji
 * - @c sensors_{id}.json – stanowiska stacji o danym id
 * - @c measurements_{id}.json – pomiary stanowiska o danym id
 *
 * Implementuje wzorzec Strategy przez interfejs @ref IDatabase.
 */
class JsonDatabase : public IDatabase {
public:
    /**
     * @brief Konstruktor.
     * @param dataDir  Ścieżka katalogu danych (zostanie utworzony jeśli nie istnieje).
     */
    explicit JsonDatabase(const QString& dataDir = ".");

    void saveStations(const std::vector<Station>& stations) override;
    std::vector<Station> loadStations() override;

    void saveSensors(int stationId, const std::vector<Sensor>& sensors) override;
    std::vector<Sensor> loadSensors(int stationId) override;

    void saveMeasurements(const Measurement& m) override;
    Measurement loadMeasurements(int sensorId) override;
    bool hasMeasurements(int sensorId) override;

private:
    QString m_dataDir; ///< Katalog przechowywania plików JSON

    /** @brief Zwraca pełną ścieżkę do pliku o podanej nazwie. */
    QString filePath(const QString& name) const;

    /** @brief Odczytuje plik JSON z dysku. */
    QByteArray readFile(const QString& path) const;

    /** @brief Zapisuje dane do pliku JSON na dysku. */
    void writeFile(const QString& path, const QByteArray& data) const;
};

} // namespace aqm
