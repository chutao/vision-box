/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Data Flow Graph Model - Bridge between QtNodes and VisionBox
 ******************************************************************************/

#ifndef VISIONBOX_DATAFLOWGRAPHMODEL_H
#define VISIONBOX_DATAFLOWGRAPHMODEL_H

#include <QtNodes/DataFlowGraphModel>
#include <QtNodes/NodeDelegateModelRegistry>
#include <QtNodes/Definitions>
#include <QUuid>
#include <QMap>
#include <QString>
#include <memory>

namespace VisionBox {

using ::QtNodes::NodeId;

class PluginManager;

/*******************************************************************************
 * DataFlowGraphModel
 *
 * Extends QtNodes::DataFlowGraphModel to integrate with VisionBox's
 * plugin system. Provides node models from loaded plugins and handles
 * graph serialization.
 ******************************************************************************/
class DataFlowGraphModel : public ::QtNodes::DataFlowGraphModel
{
public:
    DataFlowGraphModel(std::shared_ptr<PluginManager> pluginManager);
    ~DataFlowGraphModel() override;

    // Save the graph to JSON
    QJsonObject save() const override;

    // Load the graph from JSON
    void load(const QJsonObject& json) override;

    // Save a single node to JSON (overrides base to add model-name)
    QJsonObject saveNode(NodeId const nodeId) const override;

    // Get the node registry
    std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registry();

private:
    std::shared_ptr<PluginManager> m_pluginManager;
};

} // namespace VisionBox

#endif // VISIONBOX_DATAFLOWGRAPHMODEL_H
