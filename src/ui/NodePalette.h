/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Node Palette - Tool Box for Dragging Nodes into Scene
 ******************************************************************************/

#ifndef VISIONBOX_NODEPALETTE_H
#define VISIONBOX_NODEPALETTE_H

#include <QDockWidget>
#include "NodePaletteTreeWidget.h"
#include <QLineEdit>
#include <QMap>
#include <memory>

namespace QtNodes {
class NodeDelegateModelRegistry;
class DataFlowGraphicsScene;
}

namespace VisionBox {

/*******************************************************************************
 * NodePalette - Dockable widget showing available nodes
 ******************************************************************************/
class NodePalette : public QDockWidget
{
    Q_OBJECT

public:
    explicit NodePalette(std::shared_ptr<::QtNodes::NodeDelegateModelRegistry> registry,
                        ::QtNodes::DataFlowGraphicsScene* scene,
                        QWidget* parent = nullptr);
    ~NodePalette() override;

    void refresh();

private slots:
    void onFilterChanged(const QString& text);
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);

private:
    void setupUi();
    void populateTree();

    ::QtNodes::DataFlowGraphicsScene* m_scene;
    std::shared_ptr<::QtNodes::NodeDelegateModelRegistry> m_registry;

    NodePaletteTreeWidget* m_treeWidget;
    QLineEdit* m_filterEdit;

    struct NodeInfo {
        QString name;
        QString caption;
    };

    QMap<QString, QList<NodeInfo>> m_categoryNodes;
};

} // namespace VisionBox

#endif // VISIONBOX_NODEPALETTE_H
