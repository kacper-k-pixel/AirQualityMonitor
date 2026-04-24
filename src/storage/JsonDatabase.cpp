#include "JsonDatabase.h"
#include <nlohmann/json.hpp>
#include <QDir>
#include <QFile>
#include <QDateTime>

using json = nlohmann::json;

namespace aqm {

// ── Constructor ───────────────────────────────────────────────────────────────

JsonDatabase::JsonDatabase(const QString& dataDir) : m_dataDir(dataDir) {
    QDir().mkpath(dataDir);
}

QString JsonDatabase::filePath(const QString& name) const {
    return m_dataDir + QDir::separator() + name;
}

QByteArray JsonDatabase::readFile(const QString& path) const {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        throw DatabaseException(QStringLiteral("Nie można otworzyć: ") + path);
    return f.readAll();
}

void JsonDatabase::writeFile(const QString& path, const QByteArray& data) const {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        throw DatabaseException(QStringLiteral("Nie można zapisać: ") + path);
    f.write(data);
}

// ── Stations ──────────────────────────────────────────────────────────────────

void JsonDatabase::saveStations(const std::vector<Station>& stations) {
    json arr = json::array();
    for (const auto& s : stations) {
        arr.push_back({
            {"id",            s.id},
            {"stationName",   s.stationName.toStdString()},
            {"gegrLat",       s.gegrLat},
            {"gegrLon",       s.gegrLon},
            {"addressStreet", s.addressStreet.toStdString()},
            {"city", {
                {"id",   s.city.id},
                {"name", s.city.name.toStdString()},
                {"commune", {
                    {"communeName",  s.city.commune.communeName.toStdString()},
                    {"districtName", s.city.commune.districtName.toStdString()},
                    {"provinceName", s.city.commune.provinceName.toStdString()}
                }}
            }}
        });
    }
    writeFile(filePath("stations.json"),
              QByteArray::fromStdString(arr.dump(2)));
}

std::vector<Station> JsonDatabase::loadStations() {
    const auto data = readFile(filePath("stations.json"));
    try {
        const auto j = json::parse(data.toStdString());
        std::vector<Station> result;
        for (const auto& item : j) {
            Station s;
            s.id          = item.at("id").get<int>();
            s.stationName = QString::fromStdString(item.at("stationName").get<std::string>());
            s.gegrLat     = item.at("gegrLat").get<double>();
            s.gegrLon     = item.at("gegrLon").get<double>();
            s.addressStreet = QString::fromStdString(item.at("addressStreet").get<std::string>());
            s.city.id     = item.at("city").at("id").get<int>();
            s.city.name   = QString::fromStdString(item.at("city").at("name").get<std::string>());
            auto& com = item.at("city").at("commune");
            s.city.commune.communeName  = QString::fromStdString(com.at("communeName").get<std::string>());
            s.city.commune.districtName = QString::fromStdString(com.at("districtName").get<std::string>());
            s.city.commune.provinceName = QString::fromStdString(com.at("provinceName").get<std::string>());
            result.push_back(std::move(s));
        }
        return result;
    } catch (const json::exception& e) {
        throw DatabaseException(QStringLiteral("loadStations JSON: ") + e.what());
    }
}

// ── Sensors ───────────────────────────────────────────────────────────────────

void JsonDatabase::saveSensors(int stationId, const std::vector<Sensor>& sensors) {
    json arr = json::array();
    for (const auto& s : sensors) {
        arr.push_back({
            {"id",        s.id},
            {"stationId", s.stationId},
            {"param", {
                {"paramName",    s.param.paramName.toStdString()},
                {"paramFormula", s.param.paramFormula.toStdString()},
                {"paramCode",    s.param.paramCode.toStdString()},
                {"idParam",      s.param.idParam}
            }}
        });
    }
    writeFile(filePath(QStringLiteral("sensors_%1.json").arg(stationId)),
              QByteArray::fromStdString(arr.dump(2)));
}

std::vector<Sensor> JsonDatabase::loadSensors(int stationId) {
    const auto data = readFile(
        filePath(QStringLiteral("sensors_%1.json").arg(stationId)));
    try {
        const auto j = json::parse(data.toStdString());
        std::vector<Sensor> result;
        for (const auto& item : j) {
            Sensor s;
            s.id        = item.at("id").get<int>();
            s.stationId = item.at("stationId").get<int>();
            s.param.paramName    = QString::fromStdString(item.at("param").at("paramName").get<std::string>());
            s.param.paramFormula = QString::fromStdString(item.at("param").at("paramFormula").get<std::string>());
            s.param.paramCode    = QString::fromStdString(item.at("param").at("paramCode").get<std::string>());
            s.param.idParam      = item.at("param").at("idParam").get<int>();
            result.push_back(std::move(s));
        }
        return result;
    } catch (const json::exception& e) {
        throw DatabaseException(QStringLiteral("loadSensors JSON: ") + e.what());
    }
}

// ── Measurements ──────────────────────────────────────────────────────────────

void JsonDatabase::saveMeasurements(const Measurement& m) {
    json vals = json::array();
    for (const auto& v : m.values) {
        json entry;
        entry["date"] = v.date.toString(Qt::ISODate).toStdString();
        if (v.value.has_value()) entry["value"] = *v.value;
        else                      entry["value"] = nullptr;
        vals.push_back(entry);
    }
    const json j = {{"sensorId", m.sensorId}, {"key", m.key.toStdString()}, {"values", vals}};
    writeFile(filePath(QStringLiteral("measurements_%1.json").arg(m.sensorId)),
              QByteArray::fromStdString(j.dump(2)));
}

Measurement JsonDatabase::loadMeasurements(int sensorId) {
    const auto data = readFile(
        filePath(QStringLiteral("measurements_%1.json").arg(sensorId)));
    try {
        const auto j = json::parse(data.toStdString());
        Measurement m;
        m.sensorId = j.at("sensorId").get<int>();
        m.key      = QString::fromStdString(j.at("key").get<std::string>());
        for (const auto& v : j.at("values")) {
            MeasurementValue mv;
            mv.date = QDateTime::fromString(
                QString::fromStdString(v.at("date").get<std::string>()), Qt::ISODate);
            if (!v.at("value").is_null())
                mv.value = v.at("value").get<double>();
            m.values.push_back(std::move(mv));
        }
        return m;
    } catch (const json::exception& e) {
        throw DatabaseException(QStringLiteral("loadMeasurements JSON: ") + e.what());
    }
}

bool JsonDatabase::hasMeasurements(int sensorId) {
    return QFile::exists(
        filePath(QStringLiteral("measurements_%1.json").arg(sensorId)));
}

} // namespace aqm
