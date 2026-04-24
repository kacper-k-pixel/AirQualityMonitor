#pragma once
#include <vector>
#include <stdexcept>
#include <QString>
#include "model/Station.h"
#include "model/Sensor.h"
#include "model/Measurement.h"

namespace aqm {

/**
 * @brief Wyjątek zgłaszany przy błędach odczytu/zapisu bazy danych.
 */
class DatabaseException : public std::runtime_error {
public:
    /** @param msg  Opis błędu. */
    explicit DatabaseException(const QString& msg)
        : std::runtime_error(msg.toStdString()) {}
};

/**
 * @brief Abstrakcyjny interfejs lokalnej bazy danych (wzorzec Strategy).
 *
 * Konkretne implementacje mogą korzystać z formatu JSON, XML lub YAML.
 * Aktualnie dostępna implementacja: @ref JsonDatabase.
 */
class IDatabase {
public:
    virtual ~IDatabase() = default;

    // ── Stations ──────────────────────────────────────────────────────────────

    /** @brief Zapisuje listę stacji do bazy. */
    virtual void saveStations(const std::vector<Station>& stations) = 0;

    /** @brief Wczytuje listę stacji z bazy. */
    virtual std::vector<Station> loadStations() = 0;

    // ── Sensors ───────────────────────────────────────────────────────────────

    /**
     * @brief Zapisuje stanowiska danej stacji.
     * @param stationId  Identyfikator stacji.
     * @param sensors    Lista stanowisk.
     */
    virtual void saveSensors(int stationId,
                             const std::vector<Sensor>& sensors) = 0;

    /**
     * @brief Wczytuje stanowiska danej stacji.
     * @param stationId  Identyfikator stacji.
     */
    virtual std::vector<Sensor> loadSensors(int stationId) = 0;

    // ── Measurements ──────────────────────────────────────────────────────────

    /**
     * @brief Zapisuje dane pomiarowe stanowiska.
     * @param m  Seria pomiarowa.
     */
    virtual void saveMeasurements(const Measurement& m) = 0;

    /**
     * @brief Wczytuje dane pomiarowe stanowiska.
     * @param sensorId  Identyfikator stanowiska.
     */
    virtual Measurement loadMeasurements(int sensorId) = 0;

    /**
     * @brief Sprawdza, czy istnieją zapisane dane dla stanowiska.
     * @param sensorId  Identyfikator stanowiska.
     */
    virtual bool hasMeasurements(int sensorId) = 0;
};

} // namespace aqm