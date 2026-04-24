#include "Analyzer.h"
#include <algorithm>
#include <numeric>
#include <cmath>

namespace aqm {

std::optional<AnalysisResult> Analyzer::analyze(const Measurement& m) {
    std::vector<MeasurementValue> valid;
    valid.reserve(m.values.size());
    for (const auto& v : m.values)
        if (v.value.has_value()) valid.push_back(v);

    if (valid.empty()) return std::nullopt;

    AnalysisResult r;
    r.validCount = static_cast<int>(valid.size());

    // Min / max
    const auto [itMin, itMax] = std::minmax_element(
        valid.begin(), valid.end(),
        [](const MeasurementValue& a, const MeasurementValue& b) {
            return *a.value < *b.value;
        });
    r.minValue = *itMin->value;
    r.minDate  = itMin->date;
    r.maxValue = *itMax->value;
    r.maxDate  = itMax->date;

    // Średnia arytmetyczna
    const double sum = std::accumulate(valid.begin(), valid.end(), 0.0,
        [](double acc, const MeasurementValue& v) { return acc + *v.value; });
    r.average = sum / static_cast<double>(valid.size());

    // Współczynnik trendu (regresja liniowa OLS)
    r.trendSlope = computeTrendSlope(valid);

    return r;
}

Measurement Analyzer::filterByDateRange(const Measurement& m,
                                         const QDateTime& from,
                                         const QDateTime& to) {
    Measurement filtered;
    filtered.sensorId = m.sensorId;
    filtered.key      = m.key;
    for (const auto& v : m.values)
        if (v.date >= from && v.date <= to)
            filtered.values.push_back(v);
    return filtered;
}

double Analyzer::computeTrendSlope(const std::vector<MeasurementValue>& values) {
    std::vector<std::pair<double,double>> pts;
    pts.reserve(values.size());
    for (const auto& v : values)
        if (v.value.has_value() && v.date.isValid())
            pts.emplace_back(
                static_cast<double>(v.date.toSecsSinceEpoch()),
                *v.value);

    const int n = static_cast<int>(pts.size());
    if (n < 2) return 0.0;

    // Metoda najmniejszych kwadratów (OLS):
    // slope = (n·Σxy − Σx·Σy) / (n·Σx² − (Σx)²)
    double sumX{}, sumY{}, sumXY{}, sumX2{};
    for (const auto& [x, y] : pts) {
        sumX  += x;
        sumY  += y;
        sumXY += x * y;
        sumX2 += x * x;
    }
    const double denom = n * sumX2 - sumX * sumX;
    if (std::abs(denom) < 1e-12) return 0.0;
    return (n * sumXY - sumX * sumY) / denom;
}

} // namespace aqm