/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Machine Learning Plugin
 ******************************************************************************/

#ifndef VISIONBOX_MACHINELEARNINGPLUGIN_H
#define VISIONBOX_MACHINELEARNINGPLUGIN_H

#include "core/PluginInterface.h"
#include <QString>
#include <QStringList>

namespace VisionBox {

/*******************************************************************************
 * MachineLearningPlugin - Provides ML and DNN inference nodes
 ******************************************************************************/
class MachineLearningPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    MachineLearningPlugin() = default;
    ~MachineLearningPlugin() override = default;

    // Plugin identification
    QString pluginId() const override
    {
        return "machinelearning";
    }

    QString pluginName() const override
    {
        return "Machine Learning Plugin";
    }

    QString pluginVersion() const override
    {
        return "1.0.0";
    }

    QString pluginDescription() const override
    {
        return "Provides DNN inference and object tracking";
    }

    QString pluginAuthor() const override
    {
        return "VisionBox Team";
    }

    QStringList categories() const override
    {
        return QStringList() << "Machine Learning";
    }

    // Node model creation
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>>
        createNodeModels() const override;

    // Lifecycle
    void initialize() override;
    void cleanup() override;
};

} // namespace VisionBox

#endif // VISIONBOX_MACHINELEARNINGPLUGIN_H
