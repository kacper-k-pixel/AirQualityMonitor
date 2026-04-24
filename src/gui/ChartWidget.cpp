#include "ChartWidget.h"
#include "core/Analyzer.h"
#include <QVBoxLayout>
#include <QDateTime>
#include <algorithm>

namespace aqm {

ChartWidget::ChartWidget(QWidget* parent) : QWidget(parent) {
    m_chart     = new QChart();
    m_series    = new QLineSeries(m_chart);
    m_axisX     = new QDateTimeAxis(m_chart);
    m_axisY     = new QValueAxis(m_chart);
    m_chartView = new QChartView(m_chart, this);

    m_chart->setTitle(QStringLiteral("Dane pomiarowe"));
    m_chart->setAnimationOptions(QChart::SeriesAnimations);
    m_chart->legend()->hide();
    m_chart->addSeries(m_series);

    m_axisX->setFormat(QStringLiteral("dd.MM\nhh:mm"));
    m_axisX->setTitleText(QStringLiteral("Data i czas"));
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_series->attachAxis(m_axisX);

    m_axisY->setTitleText(QStringLiteral("Wartość"));
    m_axisY->setLabelFormat(QStringLiteral("%.2f"));
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisY);

    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_chartView);
}

void ChartWidget::setMeasurement(const Measurement& m) {
    m_data = m;
    if (!m.values.empty()) {
        m_from = m.values.back().date;  // najstarsza = początek zakresu
        m_to   = m.values.front().date; // najnowsza  = koniec zakresu
    }
    m_chart->setTitle(QStringLiteral("Parametr: %1").arg(m.key));
    rebuildChart();
}

void ChartWidget::setDateRange(const QDateTime& from, const QDateTime& to) {
    m_from = from;
    m_to   = to;
    rebuildChart();
}

void ChartWidget::clear() {
    m_series->clear();
    m_data = {};
    m_chart->setTitle(QStringLiteral("Dane pomiarowe"));
}

void ChartWidget::rebuildChart() {
    m_series->clear();

    const Measurement filtered = (m_from.isValid() && m_to.isValid())
        ? Analyzer::filterByDateRange(m_data, m_from, m_to)
        : m_data;

    QList<QPointF> points;
    for (const auto& v : filtered.values) {
        if (!v.value.has_value() || !v.date.isValid()) continue;
        points.append(QPointF(
            static_cast<double>(v.date.toMSecsSinceEpoch()),
            *v.value));
    }

    if (points.isEmpty()) return;

    // API zwraca dane malejąco po czasie – sortujemy rosnąco dla Qt Charts
    std::sort(points.begin(), points.end(),
              [](const QPointF& a, const QPointF& b) {
                  return a.x() < b.x();
              });

    m_series->replace(points);

    const QDateTime xMin = QDateTime::fromMSecsSinceEpoch(
        static_cast<qint64>(points.first().x()));
    const QDateTime xMax = QDateTime::fromMSecsSinceEpoch(
        static_cast<qint64>(points.last().x()));
    m_axisX->setRange(xMin, xMax);

    double yMin = points[0].y();
    double yMax = points[0].y();
    for (const auto& p : points) {
        yMin = std::min(yMin, p.y());
        yMax = std::max(yMax, p.y());
    }
    const double margin = (yMax - yMin) * 0.1 + 0.5;
    m_axisY->setRange(yMin - margin, yMax + margin);

    m_axisX->setTickCount(qBound(3, points.size() / 6, 12));
}

} // namespace aqm