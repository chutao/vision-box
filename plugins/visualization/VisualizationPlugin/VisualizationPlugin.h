/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Visualization Plugin Header
 ******************************************************************************/

#ifndef VISIONBOX_VISUALIZATIONPLUGIN_H
#define VISIONBOX_VISUALIZATIONPLUGIN_H

#include "core/PluginInterface.h"
#include <QString>
#include <QStringList>

namespace VisionBox {

/***************************************************************************//**
 * VisualizationPlugin - Visualization and drawing tools
 ******************************************************************************/
class VisualizationPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    VisualizationPlugin() = default;
    ~VisualizationPlugin() override = default;

    // Plugin identification
    QString pluginId() const override
    {
        return "visualization";
    }

    QString pluginName() const override
    {
        return "Visualization Plugin";
    }

    QString pluginVersion() const override
    {
        return "1.0.0";
    }

    QString pluginDescription() const override
    {
        return "Visualization tools for drawing overlays and displaying features";
    }

    QString pluginAuthor() const override
    {
        return "VisionBox Team";
    }

    QStringList categories() const override
    {
        return QStringList() << "Visualization";
    }

    // Node model creation
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>>
        createNodeModels() const override;

    // Lifecycle
    void initialize() override;
    void cleanup() override;
};

} // namespace VisionBox

#endif // VISIONBOX_VISUALIZATIONPLUGIN_H
