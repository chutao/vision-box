/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Custom Tree Widget with drag support for node creation
 ******************************************************************************/

#ifndef VISIONBOX_NODEPALETTREEWIDGET_H
#define VISIONBOX_NODEPALETTREEWIDGET_H

#include <QTreeWidget>

namespace VisionBox {

// Forward declaration
namespace QtNodes {
class DataFlowGraphicsScene;
}

/*******************************************************************************
 * NodePaletteTreeWidget - Custom tree widget that supports dragging nodes
 ******************************************************************************/
class NodePaletteTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    using QTreeWidget::QTreeWidget;

protected:
    // Start drag automatically when dragging an item
    void startDrag(Qt::DropActions supportedActions) override;
};

} // namespace VisionBox

#endif // VISIONBOX_NODEPALETTREEWIDGET_H
