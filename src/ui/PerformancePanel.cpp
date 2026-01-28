/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Performance Statistics Panel Implementation
 ******************************************************************************/

#include "PerformancePanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QJsonDocument>
#include <QJsonArray>
#include <QColor>
#include <QShowEvent>

namespace VisionBox {

PerformancePanel::PerformancePanel(QWidget* parent)
    : QWidget(parent)
    , m_table(nullptr)
    , m_sortCombo(nullptr)
    , m_summaryLabel(nullptr)
    , m_exportButton(nullptr)
    , m_clearButton(nullptr)
    , m_refreshButton(nullptr)
    , m_refreshTimer(new QTimer(this))
{
    setupUi();

    // Connect to performance monitor signals
    connect(PerformanceMonitor::instance(), &PerformanceMonitor::statsUpdated,
            this, &PerformancePanel::onStatsUpdated);
    connect(PerformanceMonitor::instance(), &PerformanceMonitor::statsCleared,
            this, &PerformancePanel::onStatsCleared);

    // Auto-refresh timer (every 2 seconds)
    connect(m_refreshTimer, &QTimer::timeout, this, &PerformancePanel::refresh);
    m_refreshTimer->start(2000);

    // Initial load
    refresh();
}

void PerformancePanel::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);

    // Top control bar
    auto* controlLayout = new QHBoxLayout();

    // Sort combo box
    controlLayout->addWidget(new QLabel("Sort by:"));
    m_sortCombo = new QComboBox();
    m_sortCombo->addItem("Average Time", 0);
    m_sortCombo->addItem("Last Time", 1);
    m_sortCombo->addItem("Execution Count", 2);
    m_sortCombo->addItem("Node Name", 3);
    m_sortCombo->setCurrentIndex(0);
    connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PerformancePanel::onSortChanged);
    controlLayout->addWidget(m_sortCombo);

    controlLayout->addStretch();

    // Buttons
    m_refreshButton = new QPushButton("Refresh");
    connect(m_refreshButton, &QPushButton::clicked, this, &PerformancePanel::refresh);
    controlLayout->addWidget(m_refreshButton);

    m_exportButton = new QPushButton("Export");
    connect(m_exportButton, &QPushButton::clicked, this, &PerformancePanel::onExportClicked);
    controlLayout->addWidget(m_exportButton);

    m_clearButton = new QPushButton("Clear");
    connect(m_clearButton, &QPushButton::clicked, this, &PerformancePanel::clearStats);
    controlLayout->addWidget(m_clearButton);

    mainLayout->addLayout(controlLayout);

    // Summary label
    m_summaryLabel = new QLabel();
    m_summaryLabel->setAlignment(Qt::AlignCenter);
    m_summaryLabel->setStyleSheet(
        "QLabel {"
        "    background-color: #3d3d3d;"
        "    color: #ffffff;"
        "    font-weight: bold;"
        "    padding: 8px;"
        "    border-radius: 4px;"
        "}"
    );
    mainLayout->addWidget(m_summaryLabel);

    // Table
    m_table = new QTableWidget();
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({
        "Node", "Caption", "Last (ms)", "Avg (ms)", "Min (ms)", "Max (ms)", "Count"
    });

    // Configure table
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->horizontalHeader()->setStretchLastSection(true);

    // Dark theme styling
    m_table->setStyleSheet(
        "QTableWidget {"
        "    background-color: #2b2b2b;"
        "    alternate-background-color: #323232;"
        "    color: #ffffff;"
        "    gridline-color: #505050;"
        "    border: 1px solid #505050;"
        "    font-size: 11px;"
        "}"
        "QTableWidget::item {"
        "    padding: 5px;"
        "    border: none;"
        "}"
        "QTableWidget::item:selected {"
        "    background-color: #4a6fa5;"
        "    color: #ffffff;"
        "}"
        "QTableWidget::item:hover {"
        "    background-color: #3a3a3a;"
        "}"
        "QHeaderView::section {"
        "    background-color: #3d3d3d;"
        "    color: #ffffff;"
        "    padding: 6px;"
        "    border: none;"
        "    border-right: 1px solid #505050;"
        "    border-bottom: 1px solid #505050;"
        "    font-weight: bold;"
        "}"
        "QTableWidget QTableCornerButton::section {"
        "    background-color: #3d3d3d;"
        "    border: 1px solid #505050;"
        "}"
        "QScrollBar:vertical {"
        "    background-color: #2b2b2b;"
        "    width: 12px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background-color: #5a5a5a;"
        "    min-height: 20px;"
        "    border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background-color: #6a6a6a;"
        "}"
        "QScrollBar:horizontal {"
        "    background-color: #2b2b2b;"
        "    height: 12px;"
        "}"
        "QScrollBar::handle:horizontal {"
        "    background-color: #5a5a5a;"
        "    min-width: 20px;"
        "    border-radius: 4px;"
        "}"
        "QScrollBar::handle:horizontal:hover {"
        "    background-color: #6a6a6a;"
        "}"
    );
    m_table->verticalHeader()->setVisible(false);

    // Set column widths
    m_table->setColumnWidth(0, 150); // Node name
    m_table->setColumnWidth(1, 120); // Caption
    m_table->setColumnWidth(2, 80);  // Last
    m_table->setColumnWidth(3, 80);  // Avg
    m_table->setColumnWidth(4, 80);  // Min
    m_table->setColumnWidth(5, 80);  // Max
    m_table->setColumnWidth(6, 60);  // Count

    mainLayout->addWidget(m_table);
}

void PerformancePanel::refresh()
{
    int sortIndex = m_sortCombo->currentData().toInt();
    QVector<PerformanceStats> stats;

    switch (sortIndex)
    {
        case 0: // Average Time
            stats = PerformanceMonitor::instance()->getSortedByAvgTime();
            break;
        case 1: // Last Time
            stats = PerformanceMonitor::instance()->getSortedByLastTime();
            break;
        case 2: // Execution Count
            stats = PerformanceMonitor::instance()->getSortedByExecutionCount();
            break;
        case 3: // Node Name (alphabetical)
        {
            stats = PerformanceMonitor::instance()->getAllStats();
            std::sort(stats.begin(), stats.end(),
                [](const PerformanceStats& a, const PerformanceStats& b)
                {
                    return a.nodeName < b.nodeName;
                });
            break;
        }
    }

    updateTable(stats);
}

void PerformancePanel::updateTable(const QVector<PerformanceStats>& stats)
{
    m_table->setRowCount(stats.size());

    double totalAvgTime = 0.0;
    int totalExecutions = 0;
    int slowNodeCount = 0;

    for (int row = 0; row < stats.size(); ++row)
    {
        const PerformanceStats& stat = stats[row];

        // Node name - use caption with instance pointer for uniqueness
        QString displayName = stat.nodeName;
        if (displayName.isEmpty())
        {
            // Use caption as the name, add instance pointer for uniqueness
            displayName = stat.nodeCaption;
        }
        // Add instance address to distinguish multiple instances of same type
        QString instanceId = QString(" (0x%1)").arg(
            reinterpret_cast<quintptr>(stat.nodeInstance), 0, 16);
        QString uniqueName = displayName + instanceId;

        auto* nameItem = new QTableWidgetItem(uniqueName);
        m_table->setItem(row, 0, nameItem);

        // Caption
        auto* captionItem = new QTableWidgetItem(stat.nodeCaption);
        m_table->setItem(row, 1, captionItem);

        // Last execution time
        auto* lastItem = new QTableWidgetItem(QString::number(stat.lastMs(), 'f', 2));
        m_table->setItem(row, 2, lastItem);

        // Average execution time
        auto* avgItem = new QTableWidgetItem(QString::number(stat.avgMs(), 'f', 2));
        m_table->setItem(row, 3, avgItem);

        // Min execution time
        auto* minItem = new QTableWidgetItem(QString::number(stat.minMs(), 'f', 2));
        m_table->setItem(row, 4, minItem);

        // Max execution time
        auto* maxItem = new QTableWidgetItem(QString::number(stat.maxMs(), 'f', 2));
        m_table->setItem(row, 5, maxItem);

        // Execution count
        auto* countItem = new QTableWidgetItem(QString::number(stat.executionCount));
        m_table->setItem(row, 6, countItem);

        // Color coding for slow nodes
        QString color = getPerformanceColor(stat.avgMs(), stat.lastMs());
        if (!color.isEmpty())
        {
            for (int col = 0; col < 7; ++col)
            {
                if (auto* item = m_table->item(row, col))
                {
                    item->setBackground(QColor(color));
                    // Ensure white text on colored backgrounds
                    item->setForeground(QColor("#ffffff"));
                }
            }
            slowNodeCount++;
        }

        // Statistics
        totalAvgTime += stat.avgMs();
        totalExecutions += stat.executionCount;
    }

    // Update summary
    QString summary;
    if (stats.isEmpty())
    {
        summary = "No performance data available";
    }
    else
    {
        double overallAvg = totalAvgTime / stats.size();
        summary = QString("Nodes: %1 | Total Executions: %2 | Overall Avg: %3 ms")
                       .arg(stats.size())
                       .arg(totalExecutions)
                       .arg(overallAvg, 0, 'f', 2);

        if (slowNodeCount > 0)
        {
            summary += QString(" | Slow Nodes (>100ms): %1").arg(slowNodeCount);
        }
    }

    m_summaryLabel->setText(summary);
}

QString PerformancePanel::formatTime(double milliseconds) const
{
    if (milliseconds < 1.0)
        return QString::number(milliseconds * 1000, 'f', 2) + " μs";
    else if (milliseconds < 1000.0)
        return QString::number(milliseconds, 'f', 2) + " ms";
    else
        return QString::number(milliseconds / 1000.0, 'f', 2) + " s";
}

QString PerformancePanel::getPerformanceColor(double avgMs, double lastMs) const
{
    // Dark theme color coding for performance
    // > 100ms: Dark Red (very slow)
    // > 50ms: Dark Orange (slow)
    // > 10ms: Dark Yellow (moderate)
    // < 10ms: No color (fast)

    if (avgMs > 100.0 || lastMs > 100.0)
        return "#5c1a1a"; // Dark red
    else if (avgMs > 50.0 || lastMs > 50.0)
        return "#5c3a1a"; // Dark orange
    else if (avgMs > 10.0 || lastMs > 10.0)
        return "#5c5c1a"; // Dark yellow
    else
        return QString(); // No color
}

void PerformancePanel::onStatsUpdated(const void* nodeInstance)
{
    // Refresh on next timer tick to avoid excessive updates
    Q_UNUSED(nodeInstance);
}

void PerformancePanel::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    // Immediately refresh when panel becomes visible
    refresh();
}

void PerformancePanel::onStatsCleared()
{
    refresh();
}

void PerformancePanel::onSortChanged(int index)
{
    Q_UNUSED(index);
    refresh();
}

void PerformancePanel::onExportClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export Performance Statistics",
                                                    "", "JSON Files (*.json);;All Files (*.*)");
    if (fileName.isEmpty())
        return;

    QJsonArray stats = PerformanceMonitor::instance()->toJson();
    QJsonDocument doc(stats);

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this, "Error", "Failed to open file for writing");
        return;
    }

    file.write(doc.toJson());
    file.close();

    QMessageBox::information(this, "Export Complete",
                           QString("Exported %1 node statistics to %2")
                           .arg(stats.size())
                           .arg(fileName));
}

void PerformancePanel::clearStats()
{
    auto reply = QMessageBox::question(this, "Clear Statistics",
                                      "Are you sure you want to clear all performance statistics?",
                                      QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        PerformanceMonitor::instance()->clear();
    }
}

} // namespace VisionBox
