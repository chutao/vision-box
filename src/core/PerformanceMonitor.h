/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Performance Monitoring and Statistics
 ******************************************************************************/

#ifndef VISIONBOX_PERFORMANCE_MONITOR_H
#define VISIONBOX_PERFORMANCE_MONITOR_H

#include <QObject>
#include <QString>
#include <QElapsedTimer>
#include <QMap>
#include <QVector>
#include <QMutex>
#include <QJsonObject>

namespace VisionBox {

/**
 * @brief Performance statistics for a single node execution
 */
struct PerformanceStats
{
    QString nodeName;           // Node name
    QString nodeCaption;        // Node caption (display name)
    void* nodeInstance;         // Node instance pointer (for unique identification)
    qint64 lastExecutionTime;   // Last execution time (microseconds)
    qint64 avgExecutionTime;    // Average execution time
    qint64 minExecutionTime;    // Minimum execution time
    qint64 maxExecutionTime;    // Maximum execution time
    qint64 totalExecutionTime;  // Total execution time
    int executionCount;         // Number of executions

    PerformanceStats()
        : nodeName()
        , nodeCaption()
        , nodeInstance(nullptr)
        , lastExecutionTime(0)
        , avgExecutionTime(0)
        , minExecutionTime(0)
        , maxExecutionTime(0)
        , totalExecutionTime(0)
        , executionCount(0)
    {}

    // Get execution time in milliseconds
    double lastMs() const { return lastExecutionTime / 1000.0; }
    double avgMs() const { return avgExecutionTime / 1000.0; }
    double minMs() const { return minExecutionTime / 1000.0; }
    double maxMs() const { return maxExecutionTime / 1000.0; }

    // Convert to JSON
    QJsonObject toJson() const
    {
        QJsonObject obj;
        obj["nodeName"] = nodeName;
        obj["nodeCaption"] = nodeCaption;
        obj["lastMs"] = lastMs();
        obj["avgMs"] = avgMs();
        obj["minMs"] = minMs();
        obj["maxMs"] = maxMs();
        obj["executionCount"] = executionCount;
        return obj;
    }

    // Get formatted time string
    QString formattedLastMs() const
    {
        return QString::number(lastMs(), 'f', 2) + " ms";
    }

    QString formattedAvgMs() const
    {
        return QString::number(avgMs(), 'f', 2) + " ms";
    }
};

/**
 * @brief RAII-style performance timer
 *
 * Usage:
 * {
 *     PerformanceTimer timer(this, "My Caption");
 *     // ... do work ...
 * } // Timer automatically records elapsed time on destruction
 */
class PerformanceTimer
{
public:
    PerformanceTimer(const void* nodeInstance, const QString& nodeCaption);
    ~PerformanceTimer();

    // Disable copy
    PerformanceTimer(const PerformanceTimer&) = delete;
    PerformanceTimer& operator=(const PerformanceTimer&) = delete;

    qint64 elapsed() const { return m_timer.elapsed(); }

private:
    const void* m_nodeInstance;  // Node instance pointer for unique identification
    QString m_nodeCaption;
    QElapsedTimer m_timer;
};

/**
 * @brief Global performance monitor
 *
 * Singleton that collects performance statistics from all nodes.
 * Thread-safe for concurrent node execution.
 */
class PerformanceMonitor : public QObject
{
    Q_OBJECT

public:
    static PerformanceMonitor* instance();

    // Record execution time
    void recordExecution(const void* nodeInstance,
                        const QString& nodeName,
                        const QString& nodeCaption,
                        qint64 elapsedMicroseconds);

    // Get all statistics
    QVector<PerformanceStats> getAllStats() const;

    // Clear all statistics
    void clear();

    // Check if monitoring is enabled
    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled) { m_enabled = enabled; }

    // Get statistics sorted by execution time
    QVector<PerformanceStats> getSortedByAvgTime() const;
    QVector<PerformanceStats> getSortedByLastTime() const;
    QVector<PerformanceStats> getSortedByExecutionCount() const;

    // Export statistics to JSON
    QJsonArray toJson() const;

signals:
    void statsUpdated(const void* nodeInstance);
    void statsCleared();

private:
    PerformanceMonitor();
    ~PerformanceMonitor() = default;

    mutable QMutex m_mutex;
    QMap<const void*, PerformanceStats> m_stats;  // Use pointer as key
    bool m_enabled;

    // Prevent copy
    PerformanceMonitor(const PerformanceMonitor&) = delete;
    PerformanceMonitor& operator=(const PerformanceMonitor&) = delete;
};

} // namespace VisionBox

#endif // VISIONBOX_PERFORMANCE_MONITOR_H
