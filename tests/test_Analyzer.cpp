#include <gtest/gtest.h>
#include "core/Analyzer.h"

using namespace aqm;

// ── Helpers ───────────────────────────────────────────────────────────────────

static Measurement makeMeasurement(
        const std::vector<std::pair<QString, std::optional<double>>>& data) {
    Measurement m;
    m.key      = "TEST";
    m.sensorId = 1;
    for (const auto& [dateStr, val] : data) {
        MeasurementValue mv;
        mv.date  = QDateTime::fromString(dateStr, Qt::ISODate);
        mv.value = val;
        m.values.push_back(mv);
    }
    return m;
}

// ── analyze ───────────────────────────────────────────────────────────────────

TEST(AnalyzerTest, ReturnsNulloptForEmptyMeasurement) {
    Measurement m;
    EXPECT_FALSE(Analyzer::analyze(m).has_value());
}

TEST(AnalyzerTest, ReturnsNulloptForAllNullValues) {
    const auto m = makeMeasurement({
        {"2024-01-01T10:00:00", std::nullopt},
        {"2024-01-01T11:00:00", std::nullopt}
    });
    EXPECT_FALSE(Analyzer::analyze(m).has_value());
}

TEST(AnalyzerTest, CalculatesMinMaxAverage) {
    const auto m = makeMeasurement({
        {"2024-01-01T12:00:00", 10.0},
        {"2024-01-01T11:00:00", 50.0},
        {"2024-01-01T10:00:00", 30.0}
    });

    const auto r = Analyzer::analyze(m);
    ASSERT_TRUE(r.has_value());
    EXPECT_NEAR(r->minValue, 10.0, 1e-9);
    EXPECT_NEAR(r->maxValue, 50.0, 1e-9);
    EXPECT_NEAR(r->average,  30.0, 1e-9);
    EXPECT_EQ(r->validCount, 3);
}

TEST(AnalyzerTest, IgnoresNullsInCalculations) {
    const auto m = makeMeasurement({
        {"2024-01-01T12:00:00", 20.0},
        {"2024-01-01T11:00:00", std::nullopt},
        {"2024-01-01T10:00:00", 40.0}
    });

    const auto r = Analyzer::analyze(m);
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(r->validCount, 2);
    EXPECT_NEAR(r->minValue, 20.0, 1e-9);
    EXPECT_NEAR(r->maxValue, 40.0, 1e-9);
    EXPECT_NEAR(r->average,  30.0, 1e-9);
}

TEST(AnalyzerTest, SingleValueAnalysis) {
    const auto m = makeMeasurement({{"2024-01-01T10:00:00", 42.0}});
    const auto r = Analyzer::analyze(m);
    ASSERT_TRUE(r.has_value());
    EXPECT_NEAR(r->minValue, 42.0, 1e-9);
    EXPECT_NEAR(r->maxValue, 42.0, 1e-9);
    EXPECT_NEAR(r->average,  42.0, 1e-9);
}

// ── computeTrendSlope ─────────────────────────────────────────────────────────

TEST(AnalyzerTest, DetectsRisingTrend) {
    const auto m = makeMeasurement({
        {"2024-01-01T10:00:00", 10.0},
        {"2024-01-01T11:00:00", 20.0},
        {"2024-01-01T12:00:00", 30.0}
    });
    // Wartości posortowane najnowsze pierwsze – slope liczone przez czas
    const double slope = Analyzer::computeTrendSlope(m.values);
    EXPECT_GT(slope, 0.0); // trend rosnący
}

TEST(AnalyzerTest, DetectsFallingTrend) {
    const auto m = makeMeasurement({
        {"2024-01-01T12:00:00", 10.0},
        {"2024-01-01T11:00:00", 20.0},
        {"2024-01-01T10:00:00", 30.0}
    });
    const double slope = Analyzer::computeTrendSlope(m.values);
    EXPECT_LT(slope, 0.0);
}

TEST(AnalyzerTest, ReturnZeroSlopeForSinglePoint) {
    const auto m = makeMeasurement({{"2024-01-01T10:00:00", 15.0}});
    EXPECT_NEAR(Analyzer::computeTrendSlope(m.values), 0.0, 1e-12);
}

// ── filterByDateRange ─────────────────────────────────────────────────────────

TEST(AnalyzerTest, FilterByDateRangeKeepsOnlyMatchingValues) {
    const auto m = makeMeasurement({
        {"2024-01-01T14:00:00", 3.0},
        {"2024-01-01T13:00:00", 2.0},
        {"2024-01-01T12:00:00", 1.0},
        {"2024-01-01T11:00:00", 0.5},
    });

    const QDateTime from = QDateTime::fromString("2024-01-01T12:00:00", Qt::ISODate);
    const QDateTime to   = QDateTime::fromString("2024-01-01T13:00:00", Qt::ISODate);

    const Measurement f = Analyzer::filterByDateRange(m, from, to);
    ASSERT_EQ(f.values.size(), 2u);
    EXPECT_NEAR(*f.values[0].value, 2.0, 1e-9);
    EXPECT_NEAR(*f.values[1].value, 1.0, 1e-9);
}

TEST(AnalyzerTest, FilterByDateRangeReturnsEmptyOutsideRange) {
    const auto m = makeMeasurement({
        {"2024-01-01T10:00:00", 10.0},
    });

    const QDateTime from = QDateTime::fromString("2024-01-02T00:00:00", Qt::ISODate);
    const QDateTime to   = QDateTime::fromString("2024-01-03T00:00:00", Qt::ISODate);

    const Measurement f = Analyzer::filterByDateRange(m, from, to);
    EXPECT_TRUE(f.values.empty());
}
