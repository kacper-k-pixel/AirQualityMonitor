#pragma once
#include <QMainWindow>
#include <QTabWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QDateTimeEdit>
#include <vector>
#include "model/Station.h"
#include "model/Sensor.h"
#include "model/Measurement.h"
#include "model/AirQualityIndex.h"
#include "gui/ChartWidget.h"
#include "gui/AnalysisWidget.h"
#include "gui/MapWidget.h"

namespace aqm {

/**
 * @brief Główne okno aplikacji.
 *
 * Składa się z pięciu zakładek:
 * - **Stacje** – wyszukiwanie i wybór stacji pomiarowej
 * - **Stanowiska** – wybór czujnika i operacje na danych
 * - **Wykres** – wizualizacja danych w Qt Charts
 * - **Analiza** – wyniki statystyczne (min/max/średnia/trend)
 * - **Mapa** – punkty stacji kodowane kolorem indeksu AQI
 *
 * Łączy sygnały @ref DataController z widgetami GUI.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    /**
     * @brief Konstruktor – buduje GUI i podłącza sygnały/sloty.
     * @param parent  Obiekt nadrzędny Qt.
     */
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    // ── Stacje ────────────────────────────────────────────────────────────────
    void onLoadAllStations();
    void onSearchByCity();
    void onSearchByRadius();
    void onStationSelected(QListWidgetItem* item);
    void onStationsReady(std::vector<aqm::Station> stations);

    // ── Stanowiska i pomiary ──────────────────────────────────────────────────
    void onSensorSelected(QListWidgetItem* item);
    void onSensorsReady(std::vector<aqm::Sensor> sensors);
    void onLoadMeasurements();
    void onSaveMeasurements();
    void onLoadFromDb();
    void onMeasurementsReady(aqm::Measurement m);

    // ── AQI ───────────────────────────────────────────────────────────────────
    void onAirQualityIndexReady(aqm::AirQualityIndex idx);

    // ── Błędy / status ────────────────────────────────────────────────────────
    void onError(const QString& msg);
    void onStatusMessage(const QString& msg);

private:
    void setupStationsTab();
    void setupSensorsTab();
    void setupChartTab();
    void setupAnalysisTab();
    void setupMapTab();

    void populateStationList(const std::vector<Station>& stations);

    // ── Widgety ───────────────────────────────────────────────────────────────
    QTabWidget* m_tabs;

    // Zakładka Stacje
    QListWidget*    m_stationList;
    QLineEdit*      m_cityFilter;
    QLineEdit*      m_addressEdit;
    QDoubleSpinBox* m_radiusSpin;

    // Zakładka Stanowiska
    QListWidget* m_sensorList;
    QLabel*      m_stationInfoLabel;

    // Zakładka Wykres
    ChartWidget*   m_chartWidget;
    QDateTimeEdit* m_chartFromEdit; ///< Wybór początku zakresu wykresu
    QDateTimeEdit* m_chartToEdit;   ///< Wybór końca zakresu wykresu

    // Zakładka Analiza
    AnalysisWidget* m_analysisWidget;

    // Zakładka Mapa
    MapWidget* m_mapWidget;

    // ── Stan aplikacji ────────────────────────────────────────────────────────
    std::vector<Station> m_allStations;       ///< Pełna lista stacji
    std::vector<Sensor>  m_sensors;           ///< Stanowiska wybranej stacji
    Measurement          m_currentMeasurement;///< Aktualnie załadowane pomiary
    int m_selectedStationId{-1};              ///< ID wybranej stacji
    int m_selectedSensorId{-1};               ///< ID wybranego stanowiska
};

} // namespace aqm