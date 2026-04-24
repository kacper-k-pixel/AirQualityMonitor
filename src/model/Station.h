#pragma once
#include <QString>

namespace aqm {

/**
 * @brief Informacje o gminie stacji pomiarowej.
 */
struct Commune {
    QString communeName;   ///< Nazwa gminy
    QString districtName;  ///< Nazwa powiatu
    QString provinceName;  ///< Nazwa województwa
};

/**
 * @brief Informacje o mieście / miejscowości stacji.
 */
struct City {
    int     id{};    ///< Unikalny identyfikator lokalizacji
    QString name;    ///< Nazwa miejscowości
    Commune commune; ///< Przynależność administracyjna
};

/**
 * @brief Stacja pomiarowa GIOŚ.
 *
 * Odpowiada bezpośrednio strukturze zwracanej przez endpoint
 * @c /station/findAll w API GIOŚ.
 */
struct Station {
    int     id{};          ///< Unikalny identyfikator stacji
    QString stationName;   ///< Nazwa stacji
    double  gegrLat{};     ///< Szerokość geograficzna
    double  gegrLon{};     ///< Długość geograficzna
    City    city;          ///< Lokalizacja
    QString addressStreet; ///< Ulica
};

} // namespace aqm
