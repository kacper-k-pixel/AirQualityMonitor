#pragma once
#include <QByteArray>
#include <vector>
#include <stdexcept>
#include "model/Station.h"
#include "model/Sensor.h"
#include "model/Measurement.h"
#include "model/AirQualityIndex.h"

namespace aqm {

/**
 * @brief Wyjątek rzucany przy błędach parsowania JSON.
 */
class ParseException : public std::runtime_error {
public:
    /** @param msg  Opis błędu. */
    explicit ParseException(const QString& msg)
        : std::runtime_error(msg.toStdString()) {}
};

/**
 * @brief Parser odpowiedzi JSON z API GIOŚ.
 *
 * Wszystkie metody są statyczne i bezstanowe.  Każda z nich rzuca
 * @ref ParseException jeśli JSON jest niekompletny lub nieprawidłowy.
 */
class JsonParser {
public:
    /**
     * @brief Parsuje listę wszystkich stacji pomiarowych.
     * @param json  Surowe bajty odpowiedzi @c /station/findAll.
     * @return      Wektor obiektów Station.
     * @throws ParseException  Przy błędzie formatu.
     */
    static std::vector<Station> parseStations(const QByteArray& json);

    /**
     * @brief Parsuje listę stanowisk pomiarowych stacji.
     * @param json  Surowe bajty odpowiedzi @c /station/sensors/{id}.
     * @return      Wektor obiektów Sensor.
     * @throws ParseException  Przy błędzie formatu.
     */
    static std::vector<Sensor> parseSensors(const QByteArray& json);

    /**
     * @brief Parsuje serię danych pomiarowych stanowiska.
     * @param json      Surowe bajty odpowiedzi @c /data/getData/{id}.
     * @param sensorId  Identyfikator stanowiska (dołączany do wyniku).
     * @return          Obiekt Measurement z wypełnionymi wartościami.
     * @throws ParseException  Przy błędzie formatu.
     */
    static Measurement parseMeasurements(const QByteArray& json, int sensorId);

    /**
     * @brief Parsuje indeks jakości powietrza dla stacji.
     * @param json      Surowe bajty odpowiedzi @c /aqindex/getIndex/{id}.
     * @param stationId Identyfikator stacji.
     * @return          Obiekt AirQualityIndex.
     * @throws ParseException  Przy błędzie formatu.
     */
    static AirQualityIndex parseAirQualityIndex(const QByteArray& json, int stationId);

    /**
     * @brief Parsuje odpowiedź Nominatim – zwraca {lat, lon}.
     * @param json  Surowe bajty odpowiedzi Nominatim.
     * @return      Para (szerokość, długość) geograficzna.
     * @throws ParseException  Gdy nie znaleziono żadnego wyniku.
     */
    static std::pair<double,double> parseNominatimCoords(const QByteArray& json);
};

} // namespace aqm
