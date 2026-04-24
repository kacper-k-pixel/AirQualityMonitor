#pragma once
#include <QString>
#include <QDateTime>
#include <vector>
#include <optional>

namespace aqm {

/**
 * @brief Pojedynczy odczyt w serii pomiarowej.
 *
 * Wartość @c value może być pusta (std::nullopt), gdy stacja
 * nie dokonała pomiaru w danym czasie.
 */
struct MeasurementValue {
    QDateTime              date;  ///< Data i czas pomiaru
    std::optional<double>  value; ///< Wartość lub brak (null w API)
};

/**
 * @brief Seria pomiarowa dla jednego stanowiska.
 *
 * Odpowiada odpowiedzi endpointu @c /data/getData/{sensorId}.
 */
struct Measurement {
    int                          sensorId{}; ///< ID stanowiska (uzupełniane lokalnie)
    QString                      key;        ///< Kod mierzonego parametru
    std::vector<MeasurementValue> values;    ///< Odczyty chronologicznie malejąco
};

} // namespace aqm
