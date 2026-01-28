/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Performance Statistics Panel
 ******************************************************************************/

#ifndef VISIONBOX_PERFORMANCE_PANEL_H
#define VISIONBOX_PERFORMANCE_PANEL_H

#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include "core/PerformanceMonitor.h"

namespace VisionBox {

/**
 * @brief Dockable panel showing performance statistics for all nodes
 */
class PerformancePanel : public QWidget
{
    Q_OBJECT

public:
    explicit PerformancePanel(QWidget* parent = nullptr);
    ~PerformancePanel() override = default;

    void refresh();
    void clearStats();

private slots:
    void onStatsUpdated(const void* nodeInstance);
    void onStatsCleared();
    void onSortChanged(int index);
    void onExportClicked();

protected:
    void showEvent(QShowEvent* event) override;

private:
    void setupUi();
    void updateTable(const QVector<PerformanceStats>& stats);
    QString formatTime(double milliseconds) const;
    QString getPerformanceColor(double avgMs, double lastMs) const;

    // UI Components
    QTableWidget* m_table;
    QComboBox* m_sortCombo;
    QLabel* m_summaryLabel;
    QPushButton* m_exportButton;
    QPushButton* m_clearButton;
    QPushButton* m_refreshButton;

    // Auto-refresh timer
    QTimer* m_refreshTimer;
};

} // namespace VisionBox

#endif // VISIONBOX_PERFORMANCE_PANEL_H
