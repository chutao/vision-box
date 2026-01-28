/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Custom Graphics View - Disables context menu for node creation
 ******************************************************************************/

#ifndef VISIONBOX_VISIONBOXGRAPHICSVIEW_H
#define VISIONBOX_VISIONBOXGRAPHICSVIEW_H

#include <QtNodes/GraphicsView>
#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QPoint>
#include <QHash>

// Forward declarations
namespace QtNodes {
class DataFlowGraphicsScene;
using NodeId = unsigned int;
}

namespace VisionBox {

/*******************************************************************************
 * VisionBoxGraphicsView - Custom GraphicsView that supports node dropping
 ******************************************************************************/
class VisionBoxGraphicsView : public ::QtNodes::GraphicsView
{
    Q_OBJECT

public:
    using ::QtNodes::GraphicsView::GraphicsView;

protected:
    // Disable only the scene context menu, not item interactions
    void contextMenuEvent(QContextMenuEvent* event) override;

    // Drag and drop events for node creation
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    void createNodeFromDrag(const QString& modelName, const QPoint& viewPos);
    void onNodeCreated(::QtNodes::NodeId nodeId);
    void setupNodePositioning();

private:
    // Store the position for the next node to be created
    QPointF m_nextNodePosition;
    bool m_hasPendingPosition = false;
};

} // namespace VisionBox

#endif // VISIONBOX_VISIONBOXGRAPHICSVIEW_H
