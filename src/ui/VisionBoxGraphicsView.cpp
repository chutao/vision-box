/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Custom Graphics View Implementation
 ******************************************************************************/

#include "VisionBoxGraphicsView.h"
#include <QtNodes/DataFlowGraphicsScene>
#include <QMimeData>
#include <QDebug>
#include <QTimer>

namespace VisionBox {

void VisionBoxGraphicsView::contextMenuEvent(QContextMenuEvent* event)
{
    // Check if there's an item under cursor - if so, let the item handle it
    if (itemAt(event->pos()))
    {
        // Let parent class handle context menu for items (nodes, connections)
        ::QtNodes::GraphicsView::contextMenuEvent(event);
        return;
    }

    // For empty scene area, don't show the node creation menu
    // Users should use the Node Palette instead
    event->accept();
}

void VisionBoxGraphicsView::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("application/x-visionbox-node"))
    {
        event->acceptProposedAction();
    }
    else
    {
        ::QtNodes::GraphicsView::dragEnterEvent(event);
    }
}

void VisionBoxGraphicsView::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasFormat("application/x-visionbox-node"))
    {
        event->acceptProposedAction();
    }
    else
    {
        ::QtNodes::GraphicsView::dragMoveEvent(event);
    }
}

void VisionBoxGraphicsView::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasFormat("application/x-visionbox-node"))
    {
        // Get the node model name from mime data
        QByteArray data = event->mimeData()->data("application/x-visionbox-node");
        QString modelName = QString::fromUtf8(data);

        if (!modelName.isEmpty())
        {
            // Create node at drop position
            QPoint dropPos = event->position().toPoint();
            createNodeFromDrag(modelName, dropPos);
            event->acceptProposedAction();
        }
    }
    else
    {
        ::QtNodes::GraphicsView::dropEvent(event);
    }
}

void VisionBoxGraphicsView::createNodeFromDrag(const QString& modelName, const QPoint& viewPos)
{
    // Get the scene
    auto* scene = dynamic_cast<::QtNodes::DataFlowGraphicsScene*>(this->scene());
    if (!scene)
    {
        return;
    }

    // Setup the signal connection if not already connected
    setupNodePositioning();

    // Get the graph model
    auto& graphModel = scene->graphModel();

    // Calculate target position BEFORE creating node
    QPointF scenePos = mapToScene(viewPos);
    QPointF adjustedPos = scenePos - QPointF(80, 40);

    // Store the position for the next node
    m_nextNodePosition = adjustedPos;
    m_hasPendingPosition = true;

    // Add node to the graph
    ::QtNodes::NodeId nodeId = graphModel.addNode(modelName);

    // Note: We DON'T check if nodeId == 0 here
    // QtNodes may return 0 on first call but still emit nodeCreated signal
    // The actual node will be positioned in the onNodeCreated slot

    qDebug() << "addNode returned" << nodeId << "for" << modelName << "at position" << adjustedPos;
}

void VisionBoxGraphicsView::setupNodePositioning()
{
    // Get the scene
    auto* scene = dynamic_cast<::QtNodes::DataFlowGraphicsScene*>(this->scene());
    if (!scene)
    {
        return;
    }

    // Get the graph model
    auto& graphModel = scene->graphModel();

    // Connect to nodeCreated signal (only once)
    static bool connected = false;
    if (!connected)
    {
        connect(&graphModel, &::QtNodes::AbstractGraphModel::nodeCreated,
                this, &VisionBoxGraphicsView::onNodeCreated, Qt::UniqueConnection);
        connected = true;
    }
}

void VisionBoxGraphicsView::onNodeCreated(::QtNodes::NodeId nodeId)
{
    // Check if we have a pending position for the newly created node
    if (m_hasPendingPosition)
    {
        // Get the scene
        auto* scene = dynamic_cast<::QtNodes::DataFlowGraphicsScene*>(this->scene());
        if (!scene)
        {
            return;
        }

        // Get the graph model
        auto& graphModel = scene->graphModel();

        // Set the node position
        bool success = graphModel.setNodeData(nodeId, ::QtNodes::NodeRole::Position,
                                              QVariant::fromValue(m_nextNodePosition));

        if (success) {
            qDebug() << "Successfully set position for node" << nodeId << "to" << m_nextNodePosition;
        } else {
            qWarning() << "Failed to set position for node" << nodeId;
        }

        // Clear the pending position flag
        m_hasPendingPosition = false;
    }
}

} // namespace VisionBox
