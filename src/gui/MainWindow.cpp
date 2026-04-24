#include "MainWindow.h"
#include "core/DataController.h"
#include <QDateTimeEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QStatusBar>

namespace aqm {

// ── Constructor ───────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("Monitor Jakości Powietrza – GIOŚ"));
    setMinimumSize(900, 650);
    resize(1100, 720);

    m_tabs = new QTabWidget(this);
    setCentralWidget(m_tabs);
    statusBar()->showMessage(QStringLiteral("Gotowy."));

    setupStationsTab();
    setupSensorsTab();
    setupChartTab();
    setupAnalysisTab();
    setupMapTab();

    // ── Połączenia z DataController ───────────────────────────────────────────
    auto* dc = DataController::instance();
    connect(dc, &DataController::stationsReady,
            this, &MainWindow::onStationsReady);
    connect(dc, &DataController::sensorsReady,
            this, &MainWindow::onSensorsReady);
    connect(dc, &DataController::measurementsReady,
            this, &MainWindow::onMeasurementsReady);
    connect(dc, &DataController::airQualityIndexReady,
            this, &MainWindow::onAirQualityIndexReady);
    connect(dc, &DataController::nearbyStationsReady,
            this, [this](std::vector<aqm::Station> stations) {
                // Tylko wyświetl wyniki – NIE nadpisuj m_allStations
                populateStationList(stations);
                m_mapWidget->setStations(stations);
                statusBar()->showMessage(
                    QStringLiteral("Znaleziono %1 stacji w podanym promieniu.")
                        .arg(stations.size()));
            });
    connect(dc, &DataController::errorOccurred,
            this, &MainWindow::onError);
    connect(dc, &DataController::statusMessage,
            this, &MainWindow::onStatusMessage);

    // ── Połączenie wykresu z analizą ─────────────────────────────────────────
    connect(m_analysisWidget, &AnalysisWidget::dateRangeChanged,
            m_chartWidget,    &ChartWidget::setDateRange);

    // ── Kliknięcie stacji na mapie ────────────────────────────────────────────
    connect(m_mapWidget, &MapWidget::stationClicked,
            this, [this](int id) {
                for (int i = 0; i < m_stationList->count(); ++i) {
                    if (m_stationList->item(i)->data(Qt::UserRole).toInt() == id) {
                        m_stationList->setCurrentRow(i);
                        m_tabs->setCurrentIndex(0);
                        onStationSelected(m_stationList->currentItem());
                        break;
                    }
                }
            });
}

// ── Tabs setup ────────────────────────────────────────────────────────────────

void MainWindow::setupStationsTab() {
    auto* page   = new QWidget;
    auto* layout = new QVBoxLayout(page);

    // Kontrolki wyszukiwania
    auto* searchBox = new QGroupBox(QStringLiteral("Wyszukiwanie stacji"), page);
    auto* grid      = new QGridLayout(searchBox);

    m_cityFilter = new QLineEdit(searchBox);
    m_cityFilter->setPlaceholderText(QStringLiteral("np. Poznań"));
    auto* cityBtn = new QPushButton(QStringLiteral("Filtruj po mieście"), searchBox);

    m_addressEdit = new QLineEdit(searchBox);
    m_addressEdit->setPlaceholderText(
        QStringLiteral("np. Polanka 3"));
    m_radiusSpin = new QDoubleSpinBox(searchBox);
    m_radiusSpin->setRange(1.0, 500.0);
    m_radiusSpin->setValue(25.0);
    m_radiusSpin->setSuffix(QStringLiteral(" km"));
    auto* radiusBtn = new QPushButton(QStringLiteral("Szukaj w promieniu"), searchBox);

    auto* allBtn = new QPushButton(
        QStringLiteral("Załaduj wszystkie stacje"), searchBox);

    grid->addWidget(new QLabel(QStringLiteral("Miasto:")), 0, 0);
    grid->addWidget(m_cityFilter, 0, 1);
    grid->addWidget(cityBtn,      0, 2);
    grid->addWidget(new QLabel(QStringLiteral("Adres:")), 1, 0);
    grid->addWidget(m_addressEdit, 1, 1);
    grid->addWidget(m_radiusSpin,  1, 2);
    grid->addWidget(radiusBtn,     1, 3);
    grid->addWidget(allBtn, 2, 0, 1, 4);

    m_stationList = new QListWidget(page);
    m_stationList->setAlternatingRowColors(true);

    layout->addWidget(searchBox);
    layout->addWidget(new QLabel(QStringLiteral("Lista stacji (kliknij aby wybrać):")));
    layout->addWidget(m_stationList);

    connect(allBtn,    &QPushButton::clicked, this, &MainWindow::onLoadAllStations);
    connect(cityBtn,   &QPushButton::clicked, this, &MainWindow::onSearchByCity);
    connect(radiusBtn, &QPushButton::clicked, this, &MainWindow::onSearchByRadius);
    connect(m_stationList, &QListWidget::itemDoubleClicked,
            this, &MainWindow::onStationSelected);

    m_tabs->addTab(page, QStringLiteral("Stacje"));
}

void MainWindow::setupSensorsTab() {
    auto* page   = new QWidget;
    auto* layout = new QVBoxLayout(page);

    m_stationInfoLabel = new QLabel(
        QStringLiteral("Nie wybrano stacji."), page);
    m_stationInfoLabel->setWordWrap(true);

    m_sensorList = new QListWidget(page);
    m_sensorList->setAlternatingRowColors(true);

    auto* btnLayout   = new QHBoxLayout;
    auto* loadBtn     = new QPushButton(QStringLiteral("Pobierz pomiary z API"), page);
    auto* saveBtn     = new QPushButton(QStringLiteral("Zapisz w bazie lokalnej"), page);
    auto* loadDbBtn   = new QPushButton(QStringLiteral("Wczytaj z bazy lokalnej"), page);
    btnLayout->addWidget(loadBtn);
    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(loadDbBtn);

    layout->addWidget(m_stationInfoLabel);
    layout->addWidget(new QLabel(
        QStringLiteral("Stanowiska pomiarowe (kliknij aby wybrać):")));
    layout->addWidget(m_sensorList);
    layout->addLayout(btnLayout);

    connect(m_sensorList, &QListWidget::itemClicked,
            this, &MainWindow::onSensorSelected);
    connect(loadBtn,  &QPushButton::clicked, this, &MainWindow::onLoadMeasurements);
    connect(saveBtn,  &QPushButton::clicked, this, &MainWindow::onSaveMeasurements);
    connect(loadDbBtn,&QPushButton::clicked, this, &MainWindow::onLoadFromDb);

    m_tabs->addTab(page, QStringLiteral("Stanowiska"));
}

void MainWindow::setupChartTab() {
    auto* page   = new QWidget;
    auto* layout = new QVBoxLayout(page);

    // ── Pasek wyboru okresu ───────────────────────────────────────────────────
    auto* rangeBox    = new QGroupBox(QStringLiteral("Zakres danych na wykresie"), page);
    auto* rangeLayout = new QHBoxLayout(rangeBox);

    auto* fromLabel = new QLabel(QStringLiteral("Od:"), rangeBox);
    m_chartFromEdit = new QDateTimeEdit(rangeBox);
    m_chartFromEdit->setDisplayFormat(QStringLiteral("dd.MM.yyyy HH:mm"));
    m_chartFromEdit->setCalendarPopup(true);

    auto* toLabel = new QLabel(QStringLiteral("Do:"), rangeBox);
    m_chartToEdit = new QDateTimeEdit(rangeBox);
    m_chartToEdit->setDisplayFormat(QStringLiteral("dd.MM.yyyy HH:mm"));
    m_chartToEdit->setCalendarPopup(true);

    auto* applyBtn   = new QPushButton(QStringLiteral("Zastosuj"), rangeBox);
    auto* resetBtn   = new QPushButton(QStringLiteral("Cały zakres"), rangeBox);

    rangeLayout->addWidget(fromLabel);
    rangeLayout->addWidget(m_chartFromEdit);
    rangeLayout->addWidget(toLabel);
    rangeLayout->addWidget(m_chartToEdit);
    rangeLayout->addWidget(applyBtn);
    rangeLayout->addWidget(resetBtn);
    rangeLayout->addStretch();

    // ── Wykres ────────────────────────────────────────────────────────────────
    m_chartWidget = new ChartWidget(page);

    layout->addWidget(rangeBox);
    layout->addWidget(m_chartWidget);

    // ── Połączenia ────────────────────────────────────────────────────────────
    connect(applyBtn, &QPushButton::clicked, this, [this]() {
        m_chartWidget->setDateRange(
            m_chartFromEdit->dateTime(),
            m_chartToEdit->dateTime());
    });

    connect(resetBtn, &QPushButton::clicked, this, [this]() {
        if (!m_currentMeasurement.values.empty()) {
            const QDateTime from = m_currentMeasurement.values.back().date;
            const QDateTime to   = m_currentMeasurement.values.front().date;
            m_chartFromEdit->setDateTime(from);
            m_chartToEdit->setDateTime(to);
            m_chartWidget->setDateRange(from, to);
        }
    });

    m_tabs->addTab(page, QStringLiteral("Wykres"));
}

void MainWindow::setupAnalysisTab() {
    m_analysisWidget = new AnalysisWidget;
    m_tabs->addTab(m_analysisWidget, QStringLiteral("Analiza"));
}

void MainWindow::setupMapTab() {
    auto* page   = new QWidget;
    auto* layout = new QHBoxLayout(page);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(8);

    m_mapWidget = new MapWidget(page);
    layout->addWidget(m_mapWidget, 1);

    // Panel legendy po prawej
    auto* legendBox    = new QGroupBox(QStringLiteral("Indeks jakości"), page);
    auto* legendLayout = new QVBoxLayout(legendBox);
    legendBox->setFixedWidth(160);

    const QStringList labels{
        QStringLiteral("Brak danych"),
        QStringLiteral("Bardzo dobry"),
        QStringLiteral("Dobry"),
        QStringLiteral("Umiarkowany"),
        QStringLiteral("Dostateczny"),
        QStringLiteral("Zły")
    };
    const QStringList colors{
        "#a0a0a0", "#00c800", "#96e632",
        "#ffdc00", "#ff8c00", "#dc0000"
    };

    for (int i = 0; i < labels.size(); ++i) {
        auto* row    = new QHBoxLayout;
        auto* dot    = new QLabel(legendBox);
        dot->setFixedSize(14, 14);
        dot->setStyleSheet(QStringLiteral(
            "background-color: %1; border-radius: 7px; border: 1px solid #666;")
            .arg(colors[i]));
        auto* lbl = new QLabel(labels[i], legendBox);
        lbl->setWordWrap(true);
        row->addWidget(dot);
        row->addWidget(lbl);
        row->addStretch();
        legendLayout->addLayout(row);
    }
    legendLayout->addStretch();

    layout->addWidget(legendBox, 0);
    m_tabs->addTab(page, QStringLiteral("Mapa"));
}

// ── Station slots ─────────────────────────────────────────────────────────────

void MainWindow::onLoadAllStations() {
    DataController::instance()->fetchAllStations();
}

void MainWindow::onSearchByCity() {
    const QString city = m_cityFilter->text().trimmed();
    if (city.isEmpty()) { onLoadAllStations(); return; }

    std::vector<Station> filtered;
    for (const auto& s : m_allStations)
        if (s.city.name.contains(city, Qt::CaseInsensitive))
            filtered.push_back(s);

    if (filtered.empty()) {
        // Nie mamy jeszcze wszystkich stacji – pobierz z API
        if (m_allStations.empty()) {
            DataController::instance()->fetchAllStations();
            statusBar()->showMessage(
                QStringLiteral("Pobieranie stacji – powtórz filtr po załadowaniu."));
        } else {
            statusBar()->showMessage(
                QStringLiteral("Nie znaleziono stacji w: %1").arg(city));
        }
    } else {
        populateStationList(filtered);
        statusBar()->showMessage(
            QStringLiteral("Znaleziono %1 stacji w: %2")
                .arg(filtered.size()).arg(city));
    }
}

void MainWindow::onSearchByRadius() {
    const QString city    = m_cityFilter->text().trimmed();
    const QString address = m_addressEdit->text().trimmed();

    if (city.isEmpty()) {
        QMessageBox::warning(this,
            QStringLiteral("Brak miasta"),
            QStringLiteral("Najpierw wpisz miasto w polu 'Miasto', "
                           "a następnie podaj adres do wyszukiwania."));
        m_cityFilter->setFocus();
        return;
    }

    if (address.isEmpty()) {
        QMessageBox::warning(this,
            QStringLiteral("Brak adresu"),
            QStringLiteral("Wpisz adres do wyszukiwania w promieniu."));
        m_addressEdit->setFocus();
        return;
    }

    if (m_allStations.empty()) {
        statusBar()->showMessage(
            QStringLiteral("Najpierw załaduj listę stacji."));
        return;
    }

    // Szukaj w promieniu używając pełnego adresu: "adres, miasto"
    const QString fullAddress = address + QStringLiteral(", ") + city;
    DataController::instance()->fetchStationsNearAddress(
        fullAddress, m_radiusSpin->value(), m_allStations);
}

void MainWindow::onStationSelected(QListWidgetItem* item) {
    if (!item) return;
    m_selectedStationId = item->data(Qt::UserRole).toInt();
    m_stationInfoLabel->setText(
        QStringLiteral("Wybrana stacja: %1 (id=%2)")
            .arg(item->text()).arg(m_selectedStationId));
    m_sensorList->clear();
    m_tabs->setCurrentIndex(1); // przełącz na zakładkę Stanowiska
    DataController::instance()->fetchSensors(m_selectedStationId);
    DataController::instance()->fetchAirQualityIndex(m_selectedStationId);
}

void MainWindow::onStationsReady(std::vector<Station> stations) {
    m_allStations = stations;
    populateStationList(stations);
    m_mapWidget->setStations(stations);
}

void MainWindow::populateStationList(const std::vector<Station>& stations) {
    m_stationList->clear();
    for (const auto& s : stations) {
        auto* item = new QListWidgetItem(
            QStringLiteral("[%1] %2 – %3")
                .arg(s.id).arg(s.stationName).arg(s.city.name));
        item->setData(Qt::UserRole, s.id);
        m_stationList->addItem(item);
    }
}

// ── Sensor slots ──────────────────────────────────────────────────────────────

void MainWindow::onSensorsReady(std::vector<Sensor> sensors) {
    m_sensors = sensors;
    m_sensorList->clear();
    for (const auto& s : sensors) {
        auto* item = new QListWidgetItem(
            QStringLiteral("[%1] %2 (%3)")
                .arg(s.id)
                .arg(s.param.paramName)
                .arg(s.param.paramFormula));
        item->setData(Qt::UserRole, s.id);
        m_sensorList->addItem(item);
    }
}

void MainWindow::onSensorSelected(QListWidgetItem* item) {
    if (!item) return;
    m_selectedSensorId = item->data(Qt::UserRole).toInt();
    statusBar()->showMessage(
        QStringLiteral("Wybrane stanowisko: %1").arg(item->text()));
}

void MainWindow::onLoadMeasurements() {
    if (m_selectedSensorId < 0) {
        QMessageBox::information(this, QStringLiteral("Brak wyboru"),
            QStringLiteral("Najpierw wybierz stanowisko pomiarowe."));
        return;
    }
    DataController::instance()->fetchMeasurements(m_selectedSensorId, false);
}

void MainWindow::onSaveMeasurements() {
    if (m_currentMeasurement.values.empty()) {
        QMessageBox::information(this, QStringLiteral("Brak danych"),
            QStringLiteral("Najpierw pobierz dane pomiarowe."));
        return;
    }
    DataController::instance()->saveMeasurements(m_currentMeasurement);
}

void MainWindow::onLoadFromDb() {
    if (m_selectedSensorId < 0) {
        QMessageBox::information(this, QStringLiteral("Brak wyboru"),
            QStringLiteral("Najpierw wybierz stanowisko pomiarowe."));
        return;
    }
    DataController::instance()->loadMeasurementsFromDb(m_selectedSensorId);
}

void MainWindow::onMeasurementsReady(Measurement m) {
    m_currentMeasurement = m;

    if (!m.values.empty()) {
        // values[0] = najnowszy, values.back() = najstarszy
        const QDateTime from = m.values.back().date;   // najstarsza = początek
        const QDateTime to   = m.values.front().date;  // najnowsza  = koniec
        m_chartFromEdit->setDateTime(from);
        m_chartToEdit->setDateTime(to);
    }

    m_chartWidget->setMeasurement(m);
    m_analysisWidget->setMeasurement(m);
    m_tabs->setCurrentIndex(2);
}

// ── AQI ───────────────────────────────────────────────────────────────────────

void MainWindow::onAirQualityIndexReady(AirQualityIndex idx) {
    m_mapWidget->updateAirQualityIndex(idx);
    if (idx.stIndexLevel.has_value()) {
        statusBar()->showMessage(
            QStringLiteral("Indeks jakości powietrza: %1")
                .arg(idx.stIndexLevel->indexLevelName));
    }
}

// ── Error / Status ────────────────────────────────────────────────────────────

void MainWindow::onError(const QString& msg) {
    statusBar()->showMessage(QStringLiteral("Błąd: ") + msg);
    QMessageBox::warning(this, QStringLiteral("Błąd"), msg);
}

void MainWindow::onStatusMessage(const QString& msg) {
    statusBar()->showMessage(msg);
}

} // namespace aqm
