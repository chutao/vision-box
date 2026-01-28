/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Node Palette Tree Widget Implementation
 ******************************************************************************/

#include "NodePaletteTreeWidget.h"
#include <QDrag>
#include <QMimeData>
#include <QPixmap>

namespace VisionBox {

void NodePaletteTreeWidget::startDrag(Qt::DropActions supportedActions)
{
    Q_UNUSED(supportedActions);

    QTreeWidgetItem* currentItemPtr = QTreeWidget::currentItem();
    if (!currentItemPtr || !currentItemPtr->parent())
    {
        // Not a node item (it's a category)
        return;
    }

    QString modelName = currentItemPtr->data(0, Qt::UserRole).toString();
    if (modelName.isEmpty())
    {
        return;
    }

    // Create mime data with node model name
    QMimeData* mimeData = new QMimeData;
    mimeData->setText(modelName);
    mimeData->setData("application/x-visionbox-node", modelName.toUtf8());

    // Create drag object
    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData);

    // Set pixmap for drag visualization
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::blue);
    drag->setPixmap(pixmap);

    // Execute drag - block until drop is complete
    drag->exec(Qt::CopyAction);
}

} // namespace VisionBox
