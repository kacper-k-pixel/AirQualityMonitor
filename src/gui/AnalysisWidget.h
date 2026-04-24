#pragma once
#include <QWidget>
#include <QDateTime>
#include "model/Measurement.h"
#include "core/Analyzer.h"

class QDateTimeEdit;
class QPushButton;
class QTableWidget;

namespace aqm {

/**
 * @brief Widget prezentujący wyniki analizy statystycznej serii pomiarowej.
 *
 * Wyświetla: minimum, maksimum (z datami), średnią oraz kierunek trendu.
 * Użytkownik może zawęzić zakres analizy przez wybór dat.
 */
class AnalysisWidget : public QWidget {
    Q_OBJECT
public:
    /**
     * @brief Konstruktor.
     * @param parent  Obiekt nadrzędny Qt.
     */
    explicit AnalysisWidget(QWidget* parent = nullptr);

public slots:
    /**
     * @brief Ustawia dane do analizy i odświeża widok.
     * @param m  Seria pomiarowa.
     */
    void setMeasurement(const aqm::Measurement& m);

signals:
    /**
     * @brief Emitowany gdy użytkownik zmieni zakres dat.
     *
     * Podłączany do @c ChartWidget::setDateRange.
     */
    void dateRangeChanged(QDateTime from, QDateTime to);

private slots:
    void onAnalyzeClicked();

private:
    void updateTable(const AnalysisResult& r);

    QDateTimeEdit* m_fromEdit;   ///< Wybór daty początkowej
    QDateTimeEdit* m_toEdit;     ///< Wybór daty końcowej
    QPushButton*   m_analyzeBtn; ///< Przycisk "Analizuj"
    QTableWidget*  m_table;      ///< Tabela wyników

    Measurement m_data; ///< Bieżące dane pomiarowe
};

} // namespace aqm