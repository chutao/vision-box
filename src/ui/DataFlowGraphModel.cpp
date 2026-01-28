/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Data Flow Graph Model Implementation
 ******************************************************************************/

#include "DataFlowGraphModel.h"
#include "core/PluginManager.h"
#include <QtNodes/NodeDelegateModelRegistry>
#include <QtNodes/NodeDelegateModel>
#include <QtNodes/Definitions>
#include <QJsonArray>
#include <QMetaObject>
#include <QTimer>

namespace VisionBox {

/*******************************************************************************
 * Helper Functions
 ******************************************************************************/
// Helper function to build the node registry
static std::shared_ptr<QtNodes::NodeDelegateModelRegistry>
buildRegistry(std::shared_ptr<PluginManager> pluginManager)
{

    auto registry = std::make_shared<QtNodes::NodeDelegateModelRegistry>();

    // Get all node models from plugins, grouped by plugin

    // Get all loaded plugins to extract categories
    auto plugins = pluginManager->getLoadedPlugins();

    // Structure to track models with their categories
    struct ModelInfo {
        QString name;
        QString caption;
        QString category;
    };

    std::vector<ModelInfo> allModelInfos;
    int modelIndex = 0;

    // Iterate through plugins to get their models and categories
    for (const auto& pluginInfo : plugins)
    {
        // Get models from this specific plugin
        QString pluginId = pluginInfo.id;
        QString category = pluginInfo.categories.isEmpty()
            ? QStringLiteral("VisionBox")
            : pluginInfo.categories.first();

        // Load the plugin to get its models
        QObject* pluginInstance = nullptr;
        for (QPluginLoader* loader : pluginManager->getLoaders())
        {
            if (loader && loader->isLoaded())
            {
                QObject* instance = loader->instance();
                VisionBox::IVisionNodePlugin* plugin = qobject_cast<VisionBox::IVisionNodePlugin*>(instance);
                if (plugin && plugin->pluginId() == pluginId)
                {
                    pluginInstance = instance;
                    break;
                }
            }
        }

        if (!pluginInstance)
        {
            continue;
        }

        IVisionNodePlugin* plugin = qobject_cast<IVisionNodePlugin*>(pluginInstance);
        if (!plugin)
        {
            continue;
        }

        // Get models from this plugin
        auto pluginModels = plugin->createNodeModels();

        for (size_t i = 0; i < pluginModels.size(); ++i)
        {
            ModelInfo info;
            info.name = pluginModels[i]->name();
            info.caption = pluginModels[i]->caption();
            info.category = category;


            // Capture the plugin ID and model index within that plugin
            QString capturedPluginId = pluginId;
            int indexInPlugin = static_cast<int>(i);

            registry->registerModel(
                [pluginManager, capturedPluginId, indexInPlugin, name = info.name]()
                -> std::unique_ptr<QtNodes::NodeDelegateModel> {
                    // Find the plugin
                    for (QPluginLoader* loader : pluginManager->getLoaders())
                    {
                        if (!loader || !loader->isLoaded())
                        {
                            continue;
                        }

                        QObject* instance = loader->instance();
                        IVisionNodePlugin* plugin = qobject_cast<IVisionNodePlugin*>(instance);
                        if (!plugin)
                        {
                            continue;
                        }

                        if (plugin->pluginId() == capturedPluginId)
                        {
                            // Get models from this plugin
                            auto models = plugin->createNodeModels();
                            if (indexInPlugin >= 0 && indexInPlugin < static_cast<int>(models.size()))
                            {
                                return std::move(models[indexInPlugin]);
                            }
                        }
                    }

                    qWarning() << "Failed to create node instance for" << name;
                    return nullptr;
                },
                category
            );

            modelIndex++;
        }
    }

    qDebug() << "Registered" << modelIndex << "node models in registry";

    return registry;
}

/*******************************************************************************
 * Constructor / Destructor
 ******************************************************************************/
DataFlowGraphModel::DataFlowGraphModel(std::shared_ptr<PluginManager> pluginManager)
    : QtNodes::DataFlowGraphModel(buildRegistry(pluginManager))
    , m_pluginManager(pluginManager)
{
}

DataFlowGraphModel::~DataFlowGraphModel()
{
    // Qt handles cleanup
}

/*******************************************************************************
 * Registry Access
 ******************************************************************************/
std::shared_ptr<QtNodes::NodeDelegateModelRegistry> DataFlowGraphModel::registry()
{
    return dataModelRegistry();
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/

QJsonObject DataFlowGraphModel::saveNode(NodeId const nodeId) const
{
    QJsonObject nodeJson;

    nodeJson["id"] = static_cast<qint64>(nodeId);

    // Delegate to base class implementation
    QJsonObject baseJson = QtNodes::DataFlowGraphModel::saveNode(nodeId);

    // Get the internal data from the base result
    QJsonObject internalData = baseJson["internal-data"].toObject();

    // Add model-name to internal-data for loading
    // This is required by QtNodes' loadNode() implementation
    QString modelName = nodeData(nodeId, QtNodes::NodeRole::Type).toString();
    internalData["model-name"] = modelName;

    nodeJson["internal-data"] = internalData;

    // Copy position from base result
    nodeJson["position"] = baseJson["position"];

    return nodeJson;
}

QJsonObject DataFlowGraphModel::save() const
{
    // Call base class save to serialize all nodes and connections
    QJsonObject sceneJson = QtNodes::DataFlowGraphModel::save();

    return sceneJson;
}

void DataFlowGraphModel::load(const QJsonObject& json)
{
    // Clear existing nodes and connections before loading

    auto nodeIds = allNodeIds();
    size_t clearedConnections = 0;

    for (auto nodeId : nodeIds)
    {
        // Delete all connections to this node
        auto connectionIds = allConnectionIds(nodeId);
        clearedConnections += connectionIds.size();

        for (auto& connId : connectionIds)
        {
            deleteConnection(connId);
        }

        // Delete the node
        deleteNode(nodeId);
    }

    // Load nodes and connections from JSON
    QtNodes::DataFlowGraphModel::load(json);

    // Count loaded connections
    auto loadedNodeIds = allNodeIds();
    size_t loadedConnections = 0;
    for (auto nodeId : loadedNodeIds)
    {
        loadedConnections += allConnectionIds(nodeId).size();
    }
}

} // namespace VisionBox
