#include <gtest/gtest.h>
#include <QTemporaryDir>
#include "storage/JsonDatabase.h"

using namespace aqm;

// ── Fixture ───────────────────────────────────────────────────────────────────

class JsonDatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_TRUE(m_tmpDir.isValid());
        m_db = std::make_unique<JsonDatabase>(m_tmpDir.path());
    }

    QTemporaryDir             m_tmpDir;
    std::unique_ptr<JsonDatabase> m_db;
};

// ── Stations ──────────────────────────────────────────────────────────────────

TEST_F(JsonDatabaseTest, SaveAndLoadStations) {
    Station s1; s1.id = 1; s1.stationName = "Stacja A";
    s1.gegrLat = 52.4; s1.gegrLon = 16.9;
    s1.city.id = 10; s1.city.name = "Poznań";
    s1.city.commune = {"Poznań", "Poznań", "Wielkopolskie"};

    Station s2; s2.id = 2; s2.stationName = "Stacja B";
    s2.gegrLat = 50.0; s2.gegrLon = 19.9;
    s2.city.id = 20; s2.city.name = "Kraków";
    s2.city.commune = {"Kraków", "Kraków", "Małopolskie"};

    m_db->saveStations({s1, s2});
    const auto loaded = m_db->loadStations();

    ASSERT_EQ(loaded.size(), 2u);
    EXPECT_EQ(loaded[0].id, 1);
    EXPECT_EQ(loaded[0].stationName, QStringLiteral("Stacja A"));
    EXPECT_NEAR(loaded[0].gegrLat, 52.4, 1e-9);
    EXPECT_EQ(loaded[0].city.name, QStringLiteral("Poznań"));
    EXPECT_EQ(loaded[0].city.commune.provinceName, QStringLiteral("Wielkopolskie"));

    EXPECT_EQ(loaded[1].id, 2);
    EXPECT_EQ(loaded[1].city.name, QStringLiteral("Kraków"));
}

TEST_F(JsonDatabaseTest, LoadStationsThrowsWhenFileAbsent) {
    EXPECT_THROW(m_db->loadStations(), DatabaseException);
}

// ── Sensors ───────────────────────────────────────────────────────────────────

TEST_F(JsonDatabaseTest, SaveAndLoadSensors) {
    Sensor s; s.id = 42; s.stationId = 1;
    s.param = {"pył zawieszony PM10", "PM10", "PM10", 3};

    m_db->saveSensors(1, {s});
    const auto loaded = m_db->loadSensors(1);

    ASSERT_EQ(loaded.size(), 1u);
    EXPECT_EQ(loaded[0].id, 42);
    EXPECT_EQ(loaded[0].param.paramFormula, QStringLiteral("PM10"));
    EXPECT_EQ(loaded[0].param.idParam, 3);
}

TEST_F(JsonDatabaseTest, LoadSensorsThrowsForUnknownStation) {
    EXPECT_THROW(m_db->loadSensors(9999), DatabaseException);
}

// ── Measurements ──────────────────────────────────────────────────────────────

TEST_F(JsonDatabaseTest, HasMeasurementsReturnsFalseInitially) {
    EXPECT_FALSE(m_db->hasMeasurements(100));
}

TEST_F(JsonDatabaseTest, SaveAndLoadMeasurements) {
    Measurement m;
    m.sensorId = 100;
    m.key      = "PM10";

    MeasurementValue v1;
    v1.date  = QDateTime::fromString("2024-01-15T14:00:00", Qt::ISODate);
    v1.value = 42.5;

    MeasurementValue v2;
    v2.date  = QDateTime::fromString("2024-01-15T13:00:00", Qt::ISODate);
    v2.value = std::nullopt; // null

    m.values = {v1, v2};
    m_db->saveMeasurements(m);

    EXPECT_TRUE(m_db->hasMeasurements(100));

    const auto loaded = m_db->loadMeasurements(100);
    EXPECT_EQ(loaded.sensorId, 100);
    EXPECT_EQ(loaded.key, QStringLiteral("PM10"));
    ASSERT_EQ(loaded.values.size(), 2u);
    ASSERT_TRUE(loaded.values[0].value.has_value());
    EXPECT_NEAR(*loaded.values[0].value, 42.5, 1e-9);
    EXPECT_FALSE(loaded.values[1].value.has_value());
}

TEST_F(JsonDatabaseTest, OverwritesMeasurementsOnSecondSave) {
    Measurement m1; m1.sensorId = 5; m1.key = "PM25";
    MeasurementValue v1; v1.value = 10.0;
    v1.date = QDateTime::fromString("2024-01-01T10:00:00", Qt::ISODate);
    m1.values = {v1};
    m_db->saveMeasurements(m1);

    Measurement m2; m2.sensorId = 5; m2.key = "PM25";
    MeasurementValue v2; v2.value = 99.0;
    v2.date = QDateTime::fromString("2024-01-02T10:00:00", Qt::ISODate);
    m2.values = {v2};
    m_db->saveMeasurements(m2);

    const auto loaded = m_db->loadMeasurements(5);
    ASSERT_EQ(loaded.values.size(), 1u);
    EXPECT_NEAR(*loaded.values[0].value, 99.0, 1e-9);
}

TEST_F(JsonDatabaseTest, LoadMeasurementsThrowsWhenAbsent) {
    EXPECT_THROW(m_db->loadMeasurements(999), DatabaseException);
}
