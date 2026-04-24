#include "AnalysisWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>

namespace aqm {

AnalysisWidget::AnalysisWidget(QWidget* parent) : QWidget(parent) {
    // ── Zakres dat ────────────────────────────────────────────────────────────
    auto* rangeBox    = new QGroupBox(QStringLiteral("Zakres analizy"), this);
    auto* rangeLayout = new QFormLayout(rangeBox);

    m_fromEdit = new QDateTimeEdit(this);
    m_fromEdit->setDisplayFormat(QStringLiteral("dd.MM.yyyy HH:mm"));
    m_fromEdit->setCalendarPopup(true);

    m_toEdit = new QDateTimeEdit(this);
    m_toEdit->setDisplayFormat(QStringLiteral("dd.MM.yyyy HH:mm"));
    m_toEdit->setCalendarPopup(true);

    rangeLayout->addRow(QStringLiteral("Od:"), m_fromEdit);
    rangeLayout->addRow(QStringLiteral("Do:"), m_toEdit);

    m_analyzeBtn = new QPushButton(QStringLiteral("Analizuj"), this);
    connect(m_analyzeBtn, &QPushButton::clicked,
            this,         &AnalysisWidget::onAnalyzeClicked);

    // ── Tabela wyników ────────────────────────────────────────────────────────
    m_table = new QTableWidget(6, 2, this);
    m_table->setHorizontalHeaderLabels(
        {QStringLiteral("Parametr"), QStringLiteral("Wartość")});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    m_table->setAlternatingRowColors(true);

    const QStringList labels{
        QStringLiteral("Wartość minimalna"),
        QStringLiteral("Data i czas minimum"),
        QStringLiteral("Wartość maksymalna"),
        QStringLiteral("Data i czas maksimum"),
        QStringLiteral("Wartość średnia"),
        QStringLiteral("Trend zmian"),
    };
    for (int i = 0; i < labels.size(); ++i)
        m_table->setItem(i, 0, new QTableWidgetItem(labels[i]));

    // ── Layout ────────────────────────────────────────────────────────────────
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(rangeBox);
    layout->addWidget(m_analyzeBtn);
    layout->addWidget(m_table);
    layout->addStretch();
}

void AnalysisWidget::setMeasurement(const Measurement& m) {
    m_data = m;
    if (!m.values.empty()) {
        m_fromEdit->setDateTime(m.values.back().date);  // najstarsza = Od
        m_toEdit->setDateTime(m.values.front().date);   // najnowsza  = Do
    }
    onAnalyzeClicked();
}

void AnalysisWidget::onAnalyzeClicked() {
    const QDateTime from = m_fromEdit->dateTime();
    const QDateTime to   = m_toEdit->dateTime();

    emit dateRangeChanged(from, to);

    const Measurement filtered = Analyzer::filterByDateRange(m_data, from, to);
    const auto result = Analyzer::analyze(filtered);

    if (!result) {
        for (int i = 0; i < 6; ++i)
            m_table->setItem(i, 1, new QTableWidgetItem(QStringLiteral("Brak danych")));
        return;
    }
    updateTable(*result);
}

void AnalysisWidget::updateTable(const AnalysisResult& r) {
    const QString fmt = QStringLiteral("dd.MM.yyyy HH:mm");

    const QString trendDesc = (r.trendSlope > 1e-9)  ? QStringLiteral("▲ Rosnący")
                            : (r.trendSlope < -1e-9) ? QStringLiteral("▼ Malejący")
                                                      : QStringLiteral("→ Stabilny");

    const QStringList values{
        QStringLiteral("%1").arg(r.minValue, 0, 'f', 2),
        r.minDate.toString(fmt),
        QStringLiteral("%1").arg(r.maxValue, 0, 'f', 2),
        r.maxDate.toString(fmt),
        QStringLiteral("%1").arg(r.average,  0, 'f', 2),
        trendDesc
    };

    for (int i = 0; i < values.size(); ++i)
        m_table->setItem(i, 1, new QTableWidgetItem(values[i]));
}

} // namespace aqm
