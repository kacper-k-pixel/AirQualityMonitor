#pragma once
#include <QWidget>
#include <QDateTime>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include "model/Measurement.h"

namespace aqm {

/**
 * @brief Widget wyświetlający wykres liniowy danych pomiarowych.
 *
 * Używa Qt Charts (QLineSeries + QDateTimeAxis).
 * Podpięty do sygnału @c DataController::measurementsReady.
 */
class ChartWidget : public QWidget {
    Q_OBJECT
public:
    /**
     * @brief Konstruktor.
     * @param parent  Obiekt nadrzędny Qt.
     */
    explicit ChartWidget(QWidget* parent = nullptr);

public slots:
    /**
     * @brief Ustawia dane do wyświetlenia na wykresie.
     * @param m  Seria pomiarowa (wartości null są pomijane).
     */
    void setMeasurement(const aqm::Measurement& m);

    /**
     * @brief Filtruje wykres do podanego zakresu dat.
     * @param from  Początek zakresu (włącznie).
     * @param to    Koniec zakresu (włącznie).
     */
    void setDateRange(const QDateTime& from, const QDateTime& to);

    /** @brief Czyści wykres i resetuje tytuł. */
    void clear();

private:
    /** @brief Przebudowuje serię danych i zakresy osi. */
    void rebuildChart();

    QChartView*    m_chartView; ///< Widok Qt Charts
    QChart*        m_chart;     ///< Obiekt wykresu
    QLineSeries*   m_series;    ///< Seria danych
    QDateTimeAxis* m_axisX;     ///< Oś czasu (pozioma)
    QValueAxis*    m_axisY;     ///< Oś wartości (pionowa)

    Measurement m_data; ///< Bieżące dane pomiarowe
    QDateTime   m_from; ///< Aktywny zakres – początek
    QDateTime   m_to;   ///< Aktywny zakres – koniec
};

} // namespace aqm