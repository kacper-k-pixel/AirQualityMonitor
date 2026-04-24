#pragma once
#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <vector>
#include "model/Station.h"
#include "model/AirQualityIndex.h"

namespace aqm {

/**
 * @brief Widget wyświetlający mapę stacji pomiarowych w Polsce.
 *
 * Stacje rysowane są jako kolorowe punkty na obrazie mapy Polski.
 * Kolor punktu odpowiada indeksowi jakości powietrza — szczegółowa
 * skala kolorów dostępna jest w panelu legendy w @ref MainWindow.
 */
class MapWidget : public QWidget {
    Q_OBJECT
public:
    /** @brief Konstruktor. */
    explicit MapWidget(QWidget* parent = nullptr);

    /**
     * @brief Ustawia listę stacji do wyświetlenia na mapie.
     * @param stations  Lista stacji (wymagane współrzędne gegrLat/gegrLon).
     */
    void setStations(const std::vector<Station>& stations);

    /**
     * @brief Aktualizuje kolor punktu stacji na podstawie indeksu jakości.
     * @param index  Indeks jakości powietrza dla danej stacji.
     */
    void updateAirQualityIndex(const AirQualityIndex& index);

signals:
    /**
     * @brief Emitowany po kliknięciu na punkt stacji na mapie.
     * @param stationId  Identyfikator klikniętej stacji.
     */
    void stationClicked(int stationId);

private:
    /** @brief Przelicza współrzędne geograficzne na piksele sceny. */
    QPointF geoToScene(double lat, double lon) const;

    /** @brief Zwraca kolor dla poziomu indeksu (0–5). */
    static QColor indexColor(int level);

    QGraphicsView*  m_view;    ///< Widok sceny
    QGraphicsScene* m_scene;   ///< Scena graficzna

    std::vector<Station> m_stations; ///< Aktualnie wyświetlane stacje

    // Zasięg geograficzny Polski
    static constexpr double LAT_MIN = 49.0;
    static constexpr double LAT_MAX = 54.9;
    static constexpr double LON_MIN = 14.1;
    static constexpr double LON_MAX = 24.2;
    static constexpr double MAP_W   = 700.0; ///< Szerokość sceny [px]
    static constexpr double MAP_H   = 500.0; ///< Wysokość sceny [px]
protected:
    void wheelEvent(QWheelEvent* event) override;
};

} // namespace aqm
