/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Performance Monitoring and Statistics Implementation
 ******************************************************************************/

#include "PerformanceMonitor.h"
#include <QMutexLocker>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>

namespace VisionBox {

/*******************************************************************************
 * PerformanceTimer Implementation
 ******************************************************************************/
PerformanceTimer::PerformanceTimer(const void* nodeInstance, const QString& nodeCaption)
    : m_nodeInstance(nodeInstance)
    , m_nodeCaption(nodeCaption)
{
    m_timer.start();
}

PerformanceTimer::~PerformanceTimer()
{
    qint64 elapsedMicroseconds = m_timer.nsecsElapsed() / 1000; // Convert to microseconds
    // Note: nodeName is not available here, will be set from nodeInstance if needed
    PerformanceMonitor::instance()->recordExecution(m_nodeInstance, QString(), m_nodeCaption, elapsedMicroseconds);
}

/*******************************************************************************
 * PerformanceMonitor Implementation
 ******************************************************************************/
PerformanceMonitor::PerformanceMonitor()
    : QObject()
    , m_enabled(true)
{
}

PerformanceMonitor* PerformanceMonitor::instance()
{
    static PerformanceMonitor monitor;
    return &monitor;
}

void PerformanceMonitor::recordExecution(const void* nodeInstance,
                                        const QString& nodeName,
                                        const QString& nodeCaption,
                                        qint64 elapsedMicroseconds)
{
    if (!m_enabled)
        return;

    QMutexLocker locker(&m_mutex);

    PerformanceStats& stats = m_stats[nodeInstance];

    // Update node instance, name and caption
    stats.nodeInstance = const_cast<void*>(nodeInstance);
    if (stats.nodeName.isEmpty())
    {
        stats.nodeName = nodeName.isEmpty() ? nodeCaption : nodeName;
    }
    stats.nodeCaption = nodeCaption;

    // Update execution time statistics
    stats.lastExecutionTime = elapsedMicroseconds;
    stats.totalExecutionTime += elapsedMicroseconds;
    stats.executionCount++;

    if (stats.executionCount == 1)
    {
        stats.minExecutionTime = elapsedMicroseconds;
        stats.maxExecutionTime = elapsedMicroseconds;
        stats.avgExecutionTime = elapsedMicroseconds;
    }
    else
    {
        stats.minExecutionTime = std::min(stats.minExecutionTime, elapsedMicroseconds);
        stats.maxExecutionTime = std::max(stats.maxExecutionTime, elapsedMicroseconds);
        stats.avgExecutionTime = stats.totalExecutionTime / stats.executionCount;
    }

    // Emit signal with node instance
    emit statsUpdated(nodeInstance);
}

QVector<PerformanceStats> PerformanceMonitor::getAllStats() const
{
    QMutexLocker locker(&m_mutex);
    return m_stats.values().toVector();
}

void PerformanceMonitor::clear()
{
    QMutexLocker locker(&m_mutex);
    m_stats.clear();
    emit statsCleared();
}

QVector<PerformanceStats> PerformanceMonitor::getSortedByAvgTime() const
{
    QMutexLocker locker(&m_mutex);

    QVector<PerformanceStats> result = m_stats.values().toVector();
    std::sort(result.begin(), result.end(),
        [](const PerformanceStats& a, const PerformanceStats& b)
        {
            return a.avgExecutionTime > b.avgExecutionTime; // Descending order
        });

    return result;
}

QVector<PerformanceStats> PerformanceMonitor::getSortedByLastTime() const
{
    QMutexLocker locker(&m_mutex);

    QVector<PerformanceStats> result = m_stats.values().toVector();
    std::sort(result.begin(), result.end(),
        [](const PerformanceStats& a, const PerformanceStats& b)
        {
            return a.lastExecutionTime > b.lastExecutionTime; // Descending order
        });

    return result;
}

QVector<PerformanceStats> PerformanceMonitor::getSortedByExecutionCount() const
{
    QMutexLocker locker(&m_mutex);

    QVector<PerformanceStats> result = m_stats.values().toVector();
    std::sort(result.begin(), result.end(),
        [](const PerformanceStats& a, const PerformanceStats& b)
        {
            return a.executionCount > b.executionCount; // Descending order
        });

    return result;
}

QJsonArray PerformanceMonitor::toJson() const
{
    QMutexLocker locker(&m_mutex);

    QJsonArray array;
    for (const PerformanceStats& stats : m_stats)
    {
        array.append(stats.toJson());
    }

    return array;
}

} // namespace VisionBox
