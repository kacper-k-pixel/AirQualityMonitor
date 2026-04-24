#include <gtest/gtest.h>
#include "api/JsonParser.h"

using namespace aqm;

// ── parseStations ─────────────────────────────────────────────────────────────

TEST(JsonParserTest, ParsesStationList) {
    const QByteArray json = R"([
        {
            "id": 114,
            "stationName": "Poznań ul. Polanka",
            "gegrLat": "52.395",
            "gegrLon": "16.867",
            "addressStreet": "ul. Polanka 3",
            "city": {
                "id": 255,
                "name": "Poznań",
                "commune": {
                    "communeName": "Poznań",
                    "districtName": "Poznań",
                    "provinceName": "Wielkopolskie"
                }
            }
        }
    ])";

    const auto stations = JsonParser::parseStations(json);
    ASSERT_EQ(stations.size(), 1u);
    EXPECT_EQ(stations[0].id, 114);
    EXPECT_EQ(stations[0].stationName, QStringLiteral("Poznań ul. Polanka"));
    EXPECT_NEAR(stations[0].gegrLat, 52.395, 1e-6);
    EXPECT_NEAR(stations[0].gegrLon, 16.867, 1e-6);
    EXPECT_EQ(stations[0].city.name, QStringLiteral("Poznań"));
    EXPECT_EQ(stations[0].city.commune.provinceName, QStringLiteral("Wielkopolskie"));
}

TEST(JsonParserTest, ParsesEmptyStationList) {
    const auto stations = JsonParser::parseStations("[]");
    EXPECT_TRUE(stations.empty());
}

TEST(JsonParserTest, ThrowsOnInvalidStationJson) {
    EXPECT_THROW(JsonParser::parseStations("{invalid}"), ParseException);
}

TEST(JsonParserTest, HandlesNullLatLon) {
    const QByteArray json = R"([{
        "id": 1, "stationName": "Test",
        "gegrLat": null, "gegrLon": null,
        "addressStreet": null,
        "city": {"id":1,"name":"X","commune":{"communeName":"C","districtName":"D","provinceName":"P"}}
    }])";
    const auto stations = JsonParser::parseStations(json);
    ASSERT_EQ(stations.size(), 1u);
    EXPECT_NEAR(stations[0].gegrLat, 0.0, 1e-9);
}

// ── parseSensors ──────────────────────────────────────────────────────────────

TEST(JsonParserTest, ParsesSensors) {
    const QByteArray json = R"([
        {
            "id": 642,
            "stationId": 114,
            "param": {
                "paramName": "pył zawieszony PM10",
                "paramFormula": "PM10",
                "paramCode": "PM10",
                "idParam": 3
            }
        }
    ])";

    const auto sensors = JsonParser::parseSensors(json);
    ASSERT_EQ(sensors.size(), 1u);
    EXPECT_EQ(sensors[0].id, 642);
    EXPECT_EQ(sensors[0].stationId, 114);
    EXPECT_EQ(sensors[0].param.paramFormula, QStringLiteral("PM10"));
    EXPECT_EQ(sensors[0].param.idParam, 3);
}

// ── parseMeasurements ─────────────────────────────────────────────────────────

TEST(JsonParserTest, ParsesMeasurements) {
    const QByteArray json = R"({
        "Lista danych pomiarowych": [
            {"Kod stanowiska": "PM10", "Data": "2024-01-15 14:00:00", "Wartość": 42.5},
            {"Kod stanowiska": "PM10", "Data": "2024-01-15 13:00:00"},
            {"Kod stanowiska": "PM10", "Data": "2024-01-15 12:00:00", "Wartość": 38.1}
        ]
    })";

    const auto m = JsonParser::parseMeasurements(json, 642);
    EXPECT_EQ(m.sensorId, 642);
    EXPECT_EQ(m.key, QStringLiteral("PM10"));
    ASSERT_EQ(m.values.size(), 3u);
    ASSERT_TRUE(m.values[0].value.has_value());
    EXPECT_NEAR(*m.values[0].value, 42.5, 1e-9);
    EXPECT_FALSE(m.values[1].value.has_value());
    ASSERT_TRUE(m.values[2].value.has_value());
    EXPECT_NEAR(*m.values[2].value, 38.1, 1e-9);
}

TEST(JsonParserTest, ThrowsOnInvalidMeasurementJson) {
    EXPECT_THROW(
        JsonParser::parseMeasurements("not-json", 1),
        ParseException);
}

// ── parseAirQualityIndex ──────────────────────────────────────────────────────

TEST(JsonParserTest, ParsesAirQualityIndex) {
    const QByteArray json = R"({
        "AqIndex": {
            "Identyfikator stacji pomiarowej": 114,
            "Data wykonania obliczeń indeksu": "2024-01-15 14:42:27",
            "Wartość indeksu": 1,
            "Nazwa kategorii indeksu": "Dobry"
        }
    })";

    const auto idx = JsonParser::parseAirQualityIndex(json, 114);
    EXPECT_EQ(idx.stationId, 114);
    ASSERT_TRUE(idx.stIndexLevel.has_value());
    EXPECT_EQ(idx.stIndexLevel->id, 1);
    EXPECT_EQ(idx.stIndexLevel->indexLevelName, QStringLiteral("Dobry"));
}

TEST(JsonParserTest, ParsesAirQualityIndexWithNullLevel) {
    const QByteArray json = R"({
        "id": 114,
        "stCalcDate": null,
        "stIndexLevel": null,
        "stSourceDataDate": null
    })";

    const auto idx = JsonParser::parseAirQualityIndex(json, 114);
    EXPECT_FALSE(idx.stIndexLevel.has_value());
}

// ── parseNominatimCoords ──────────────────────────────────────────────────────

TEST(JsonParserTest, ParsesNominatimCoords) {
    const QByteArray json = R"([
        {"lat": "52.4069820", "lon": "16.9298837", "display_name": "Poznań"}
    ])";

    const auto [lat, lon] = JsonParser::parseNominatimCoords(json);
    EXPECT_NEAR(lat, 52.4069820, 1e-5);
    EXPECT_NEAR(lon, 16.9298837, 1e-5);
}

TEST(JsonParserTest, ThrowsOnEmptyNominatimResult) {
    EXPECT_THROW(JsonParser::parseNominatimCoords("[]"), ParseException);
}
