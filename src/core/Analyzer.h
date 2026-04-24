#pragma once
#include <optional>
#include <vector>
#include <QDateTime>
#include "model/Measurement.h"

namespace aqm {

/**
 * @brief Wyniki analizy statystycznej serii pomiarowej.
 */
struct AnalysisResult {
    double    minValue{};   ///< Wartość minimalna
    QDateTime minDate;      ///< Czas wystąpienia minimum
    double    maxValue{};   ///< Wartość maksymalna
    QDateTime maxDate;      ///< Czas wystąpienia maksimum
    double    average{};    ///< Wartość średnia arytmetyczna
    double    trendSlope{}; ///< Współczynnik trendu (OLS; > 0 rosnący, < 0 malejący)
    int       validCount{}; ///< Liczba odczytów z wartością (pomija null-e)
};

/**
 * @brief Klasa wykonująca prostą analizę statystyczną danych pomiarowych.
 *
 * Wszystkie metody są statyczne i bezstanowe – można je wywołać
 * bez tworzenia instancji klasy.
 */
class Analyzer {
public:
    /**
     * @brief Analizuje serię pomiarową.
     *
     * Pomija odczyty z brakującą wartością (std::nullopt).
     *
     * @param measurement  Seria danych do analizy.
     * @return             Wyniki analizy lub std::nullopt gdy brak danych.
     */
    static std::optional<AnalysisResult> analyze(const Measurement& measurement);

    /**
     * @brief Filtruje serię po zakresie dat.
     *
     * @param measurement  Oryginalna seria.
     * @param from         Początek zakresu (włącznie).
     * @param to           Koniec zakresu (włącznie).
     * @return             Nowa seria zawierająca tylko odczyty z podanego zakresu.
     */
    static Measurement filterByDateRange(const Measurement& measurement,
                                         const QDateTime& from,
                                         const QDateTime& to);

    /**
     * @brief Oblicza współczynnik trendu metodą regresji liniowej (OLS).
     *
     * @param values  Odczyty z wartościami (null-e są ignorowane).
     * @return        Nachylenie prostej regresji lub 0.0 gdy zbyt mało danych.
     */
    static double computeTrendSlope(const std::vector<MeasurementValue>& values);
};

} // namespace aqm