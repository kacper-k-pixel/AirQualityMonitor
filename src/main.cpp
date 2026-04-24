/**
 * @file main.cpp
 * @brief Punkt wejściowy aplikacji Monitor Jakości Powietrza.
 *
 * Inicjalizuje QApplication, tworzy DataController z bazą JSON,
 * wyświetla główne okno i uruchamia pętlę zdarzeń Qt.
 */

#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include "core/DataController.h"
#include "storage/JsonDatabase.h"
#include "gui/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("AirQualityMonitor"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));
    app.setOrganizationName(QStringLiteral("JPO2025"));

    // Katalog danych użytkownika (zapis lokalnej bazy JSON)
    const QString dataDir = QStandardPaths::writableLocation(
                                QStandardPaths::AppDataLocation)
                            + QStringLiteral("/db");
    QDir().mkpath(dataDir);

    // Wstrzyknięcie bazy danych do DataController (wzorzec Singleton + Strategy)
    auto* dc = aqm::DataController::instance(&app);
    dc->setDatabase(std::make_unique<aqm::JsonDatabase>(dataDir));

    aqm::MainWindow window;
    window.show();

    return app.exec();
}
