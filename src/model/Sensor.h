#pragma once
#include <QString>

namespace aqm {

/**
 * @brief Parametr mierzony przez stanowisko pomiarowe.
 */
struct Param {
    QString paramName;    ///< Nazwa parametru (np. "pył zawieszony PM10")
    QString paramFormula; ///< Symbol chemiczny / skrót (np. "PM10")
    QString paramCode;    ///< Kod parametru
    int     idParam{};    ///< Identyfikator parametru
};

/**
 * @brief Stanowisko (czujnik) w stacji pomiarowej.
 *
 * Odpowiada strukturze zwracanej przez endpoint
 * @c /station/sensors/{stationId}.
 */
struct Sensor {
    int   id{};        ///< Unikalny identyfikator stanowiska
    int   stationId{}; ///< Identyfikator stacji nadrzędnej
    Param param;       ///< Mierzony parametr
};

} // namespace aqm
