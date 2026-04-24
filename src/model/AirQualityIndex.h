#pragma once
#include <QString>
#include <QDateTime>
#include <optional>

namespace aqm {

/**
 * @brief Poziom indeksu jakości powietrza (skala 0–5).
 */
struct IndexLevel {
    int     id{-1};          ///< Poziom (0 = brak danych, 1–5 = jakość)
    QString indexLevelName;  ///< Tekstowy opis (np. "Bardzo dobry")
};

/**
 * @brief Indeks jakości powietrza dla stacji pomiarowej.
 *
 * Odpowiada odpowiedzi endpointu @c /aqindex/getIndex/{stationId}.
 */
struct AirQualityIndex {
    int                    stationId{};      ///< ID stacji
    QDateTime              stCalcDate;       ///< Data obliczenia indeksu
    std::optional<IndexLevel> stIndexLevel;  ///< Najgorszy indeks dla stacji
    QDateTime              stSourceDataDate; ///< Data zebrania danych źródłowych
};

} // namespace aqm
