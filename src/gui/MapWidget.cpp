#include "MapWidget.h"
#include <QPixmap>
#include <QWheelEvent>
#include <QVBoxLayout>
#include <QGraphicsEllipseItem>
#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QToolTip>

namespace aqm {

// ── Klikalny punkt stacji ─────────────────────────────────────────────────────

class StationDot : public QGraphicsEllipseItem {
public:
    StationDot(int stationId, const QString& name,
               qreal x, qreal y, MapWidget* parent)
        : QGraphicsEllipseItem(x-6, y-6, 12, 12)
        , m_stationId(stationId)
        , m_name(name)
        , m_mapWidget(parent)
    {
        setAcceptHoverEvents(true);
        setCursor(Qt::PointingHandCursor);
        setToolTip(QStringLiteral("[%1] %2").arg(stationId).arg(name));
    }
    int stationId() const { return m_stationId; }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent*) override {
        emit m_mapWidget->stationClicked(m_stationId);
    }
    void hoverEnterEvent(QGraphicsSceneHoverEvent* e) override {
        setPen(QPen(Qt::white, 2));
        setZValue(10);
        QToolTip::showText(e->screenPos(), toolTip());
    }
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override {
        setPen(QPen(Qt::darkGray, 0.5));
        setZValue(2);
    }

private:
    int        m_stationId;
    QString    m_name;
    MapWidget* m_mapWidget;
};

// ── MapWidget ─────────────────────────────────────────────────────────────────

MapWidget::MapWidget(QWidget* parent) : QWidget(parent) {
    m_scene = new QGraphicsScene(0, 0, MAP_W, MAP_H, this);
    m_view  = new QGraphicsView(m_scene, this);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setDragMode(QGraphicsView::ScrollHandDrag);
    m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    m_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_view->setBackgroundBrush(QColor(173, 216, 230)); // błękitne tło = morze

    // Tło – obraz mapy Polski wczytany z zasobów Qt
    QPixmap mapImg(QStringLiteral(":/poland.png"));
    if (!mapImg.isNull()) {
        mapImg = mapImg.scaled(
            static_cast<int>(MAP_W),
            static_cast<int>(MAP_H),
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation);
        auto* bg = m_scene->addPixmap(mapImg);
        bg->setZValue(-1);
        bg->setPos(0, 0);
    }

    // Wskazówka dla użytkownika
    auto* hint = m_scene->addText(
        QStringLiteral("Kliknij stację, aby wybrać. Scroll = zoom."));
    hint->setPos(4, MAP_H - 20);
    hint->setDefaultTextColor(Qt::darkGray);
    QFont hf;
    hf.setPointSize(7);
    hint->setFont(hf);
    hint->setZValue(6);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_view);
}

QPointF MapWidget::geoToScene(double lat, double lon) const {
    const double x = (lon - LON_MIN) / (LON_MAX - LON_MIN) * MAP_W;
    const double y = (1.0 - (lat - LAT_MIN) / (LAT_MAX - LAT_MIN)) * MAP_H;
    return {x, y};
}

QColor MapWidget::indexColor(int level) {
    switch (level) {
        case 0: return QColor(0,   200,  0);  // bardzo dobry – zielony
        case 1: return QColor(150, 230, 50);  // dobry         – jasnozielony
        case 2: return QColor(255, 220,  0);  // umiarkowany   – żółty
        case 3: return QColor(255, 140,  0);  // dostateczny   – pomarańczowy
        case 4: return QColor(220,  0,   0);  // zły           – czerwony
        default: return QColor(160, 160, 160);// brak danych   – szary
    }
}

void MapWidget::setStations(const std::vector<Station>& stations) {
    // Usuń stare punkty stacji (z-value == 2)
    const auto items = m_scene->items();
    for (auto* item : items)
        if (item->zValue() == 2)
            m_scene->removeItem(item), delete item;

    m_stations = stations;

    for (const auto& s : stations) {
        if (s.gegrLat == 0.0 && s.gegrLon == 0.0) continue;
        const QPointF pos = geoToScene(s.gegrLat, s.gegrLon);
        auto* dot = new StationDot(s.id, s.stationName,
                                   pos.x(), pos.y(), this);
        dot->setPen(QPen(Qt::darkGray, 0.5));
        dot->setBrush(QBrush(indexColor(5))); // domyślnie szary = brak danych
        dot->setZValue(2);
        m_scene->addItem(dot);
    }
}

void MapWidget::updateAirQualityIndex(const AirQualityIndex& index) {
    const int level = index.stIndexLevel.has_value()
                      ? index.stIndexLevel->id : 5;
    const QColor color = indexColor(level);

    for (auto* item : m_scene->items()) {
        auto* dot = dynamic_cast<StationDot*>(item);
        if (dot && dot->stationId() == index.stationId) {
            dot->setBrush(QBrush(color));
            break;
        }
    }
}

void MapWidget::wheelEvent(QWheelEvent* event) {
    const double factor = (event->angleDelta().y() > 0) ? 1.15 : 1.0 / 1.15;
    m_view->scale(factor, factor);
    event->accept();
}

} // namespace aqm