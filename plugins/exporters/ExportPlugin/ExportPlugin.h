/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Export Plugin Header
 ******************************************************************************/

#ifndef VISIONBOX_EXPORTPLUGIN_H
#define VISIONBOX_EXPORTPLUGIN_H

#include "core/PluginInterface.h"
#include <QString>
#include <QStringList>

namespace VisionBox {

/***************************************************************************//**
 * ExportPlugin - Plugin for exporting images, videos, and data
 ******************************************************************************/
class ExportPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    ExportPlugin() = default;
    ~ExportPlugin() override = default;

    // Plugin identification
    QString pluginId() const override
    {
        return "export";
    }

    QString pluginName() const override
    {
        return "Export Plugin";
    }

    QString pluginVersion() const override
    {
        return "1.0.0";
    }

    QString pluginDescription() const override
    {
        return "Export nodes for saving images, videos, and data to disk";
    }

    QString pluginAuthor() const override
    {
        return "VisionBox Team";
    }

    QStringList categories() const override
    {
        return QStringList() << "Exporters";
    }

    // Node model creation
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>>
        createNodeModels() const override;

    // Lifecycle
    void initialize() override;
    void cleanup() override;
};

} // namespace VisionBox

#endif // VISIONBOX_EXPORTPLUGIN_H
