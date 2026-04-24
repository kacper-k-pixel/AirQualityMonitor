#include "JsonParser.h"
#include <nlohmann/json.hpp>
#include <QDateTime>

using json = nlohmann::json;

namespace aqm {

// ── Pomocnicze funkcje konwersji ──────────────────────────────────────────────

static QString qstr(const json& j) {
    return QString::fromStdString(j.get<std::string>());
}

static double parseDoubleStr(const json& j) {
    if (j.is_string()) return std::stod(j.get<std::string>());
    return j.get<double>();
}

static QDateTime parseDateTime(const json& j) {
    if (j.is_null()) return {};
    return QDateTime::fromString(qstr(j), QStringLiteral("yyyy-MM-dd HH:mm:ss"));
}

// ── Stations ──────────────────────────────────────────────────────────────────

std::vector<Station> JsonParser::parseStations(const QByteArray& data) {
    try {
        const auto j = json::parse(data.toStdString());

        const auto& arr = j.contains("Lista stacji pomiarowych")
                          ? j.at("Lista stacji pomiarowych")
                          : j;

        std::vector<Station> result;
        result.reserve(arr.size());

        for (const auto& item : arr) {
            Station s;

            if (item.contains("Identyfikator stacji"))
                s.id = item.at("Identyfikator stacji").get<int>();
            else if (item.contains("id"))
                s.id = item.at("id").get<int>();

            if (item.contains("Nazwa stacji"))
                s.stationName = qstr(item.at("Nazwa stacji"));
            else if (item.contains("stationName"))
                s.stationName = qstr(item.at("stationName"));

            if (item.contains("WGS84 \xCF\x86 N") && !item.at("WGS84 \xCF\x86 N").is_null())
                s.gegrLat = parseDoubleStr(item.at("WGS84 \xCF\x86 N"));
            else if (item.contains("gegrLat") && !item.at("gegrLat").is_null())
                s.gegrLat = parseDoubleStr(item.at("gegrLat"));

            if (item.contains("WGS84 \xCE\xBB E") && !item.at("WGS84 \xCE\xBB E").is_null())
                s.gegrLon = parseDoubleStr(item.at("WGS84 \xCE\xBB E"));
            else if (item.contains("gegrLon") && !item.at("gegrLon").is_null())
                s.gegrLon = parseDoubleStr(item.at("gegrLon"));

            if (item.contains("Ulica") && !item.at("Ulica").is_null())
                s.addressStreet = qstr(item.at("Ulica"));
            else if (item.contains("addressStreet") && !item.at("addressStreet").is_null())
                s.addressStreet = qstr(item.at("addressStreet"));

            if (item.contains("Identyfikator miasta"))
                s.city.id = item.at("Identyfikator miasta").get<int>();
            if (item.contains("Nazwa miasta") && !item.at("Nazwa miasta").is_null())
                s.city.name = qstr(item.at("Nazwa miasta"));
            if (item.contains("Gmina") && !item.at("Gmina").is_null())
                s.city.commune.communeName = qstr(item.at("Gmina"));
            if (item.contains("Powiat") && !item.at("Powiat").is_null())
                s.city.commune.districtName = qstr(item.at("Powiat"));
            if (item.contains("Województwo") && !item.at("Województwo").is_null())
                s.city.commune.provinceName = qstr(item.at("Województwo"));

            // Fallback na zagnieżdżony format city/commune
            if (s.city.name.isEmpty() && item.contains("city")
                    && !item.at("city").is_null()) {
                const auto& c = item.at("city");
                s.city.id   = c.at("id").get<int>();
                s.city.name = qstr(c.at("name"));
                if (!c.at("commune").is_null()) {
                    s.city.commune.communeName  = qstr(c.at("commune").at("communeName"));
                    s.city.commune.districtName = qstr(c.at("commune").at("districtName"));
                    s.city.commune.provinceName = qstr(c.at("commune").at("provinceName"));
                }
            }

            result.push_back(std::move(s));
        }
        return result;
    } catch (const json::exception& e) {
        throw ParseException(QStringLiteral("parseStations: ") + e.what());
    }
}

// ── Sensors ───────────────────────────────────────────────────────────────────

std::vector<Sensor> JsonParser::parseSensors(const QByteArray& data) {
    try {
        const auto j = json::parse(data.toStdString());

        const auto& arr = j.contains("Lista stanowisk pomiarowych dla podanej stacji")
                          ? j.at("Lista stanowisk pomiarowych dla podanej stacji")
                          : j.contains("Lista stanowisk pomiarowych")
                          ? j.at("Lista stanowisk pomiarowych")
                          : j;

        std::vector<Sensor> result;
        result.reserve(arr.size());

        for (const auto& item : arr) {
            Sensor s;

            if (item.contains("Identyfikator stanowiska"))
                s.id = item.at("Identyfikator stanowiska").get<int>();
            else if (item.contains("id"))
                s.id = item.at("id").get<int>();

            if (item.contains("Identyfikator stacji"))
                s.stationId = item.at("Identyfikator stacji").get<int>();
            else if (item.contains("stationId"))
                s.stationId = item.at("stationId").get<int>();

            if (item.contains("Wskaźnik") && !item.at("Wskaźnik").is_null())
                s.param.paramName = qstr(item.at("Wskaźnik"));
            else if (item.contains("param"))
                s.param.paramName = qstr(item.at("param").at("paramName"));

            if (item.contains("Wskaźnik - wzór") && !item.at("Wskaźnik - wzór").is_null())
                s.param.paramFormula = qstr(item.at("Wskaźnik - wzór"));
            else if (item.contains("param"))
                s.param.paramFormula = qstr(item.at("param").at("paramFormula"));

            if (item.contains("Wskaźnik - kod") && !item.at("Wskaźnik - kod").is_null())
                s.param.paramCode = qstr(item.at("Wskaźnik - kod"));
            else if (item.contains("param"))
                s.param.paramCode = qstr(item.at("param").at("paramCode"));

            if (item.contains("Id wskaźnika"))
                s.param.idParam = item.at("Id wskaźnika").get<int>();
            else if (item.contains("param"))
                s.param.idParam = item.at("param").at("idParam").get<int>();

            result.push_back(std::move(s));
        }
        return result;
    } catch (const json::exception& e) {
        throw ParseException(QStringLiteral("parseSensors: ") + e.what());
    }
}

// ── Measurements ──────────────────────────────────────────────────────────────

Measurement JsonParser::parseMeasurements(const QByteArray& data, int sensorId) {
    try {
        const auto j = json::parse(data.toStdString());
        Measurement m;
        m.sensorId = sensorId;

        if (!j.contains("Lista danych pomiarowych"))
            throw ParseException(
                QStringLiteral("Brak klucza 'Lista danych pomiarowych'"));

        const auto& arr = j.at("Lista danych pomiarowych");

        if (!arr.empty() && arr[0].contains("Kod stanowiska"))
            m.key = qstr(arr[0].at("Kod stanowiska"));
        else
            m.key = QStringLiteral("PARAM");

        for (const auto& v : arr) {
            MeasurementValue mv;
            if (v.contains("Data") && !v.at("Data").is_null())
                mv.date = parseDateTime(v.at("Data"));
            if (v.contains("Wartość") && !v.at("Wartość").is_null())
                mv.value = v.at("Wartość").get<double>();
            m.values.push_back(std::move(mv));
        }

        return m;
    } catch (const json::exception& e) {
        throw ParseException(QStringLiteral("parseMeasurements: ") + e.what());
    }
}

// ── AirQualityIndex ───────────────────────────────────────────────────────────

AirQualityIndex JsonParser::parseAirQualityIndex(const QByteArray& data,
                                                  int stationId) {
    try {
        const auto j = json::parse(data.toStdString());
        AirQualityIndex idx;
        idx.stationId = stationId;

        const auto& aq = j.contains("AqIndex") ? j.at("AqIndex") : j;

        if (aq.contains("Data wykonania obliczeń indeksu")
                && !aq.at("Data wykonania obliczeń indeksu").is_null())
            idx.stCalcDate = parseDateTime(
                aq.at("Data wykonania obliczeń indeksu"));

        if (aq.contains("Wartość indeksu")
                && !aq.at("Wartość indeksu").is_null()) {
            IndexLevel lvl;
            lvl.id = aq.at("Wartość indeksu").get<int>();
            if (aq.contains("Nazwa kategorii indeksu")
                    && !aq.at("Nazwa kategorii indeksu").is_null())
                lvl.indexLevelName = qstr(aq.at("Nazwa kategorii indeksu"));
            idx.stIndexLevel = lvl;
        }

        return idx;
    } catch (const json::exception& e) {
        throw ParseException(
            QStringLiteral("parseAirQualityIndex: ") + e.what());
    }
}

// ── Nominatim ─────────────────────────────────────────────────────────────────

std::pair<double,double> JsonParser::parseNominatimCoords(
        const QByteArray& data) {
    try {
        const auto j = json::parse(data.toStdString());
        if (j.empty())
            throw ParseException(
                QStringLiteral("Nominatim: nie znaleziono lokalizacji"));
        const double lat = std::stod(j[0].at("lat").get<std::string>());
        const double lon = std::stod(j[0].at("lon").get<std::string>());
        return {lat, lon};
    } catch (const json::exception& e) {
        throw ParseException(
            QStringLiteral("parseNominatimCoords: ") + e.what());
    }
}

} // namespace aqm