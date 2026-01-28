/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Image Source Plugin
 ******************************************************************************/

#ifndef VISIONBOX_IMAGESOURCEPLUGIN_H
#define VISIONBOX_IMAGESOURCEPLUGIN_H

#include "core/PluginInterface.h"
#include <QString>
#include <QStringList>

namespace VisionBox {

/*******************************************************************************
 * ImageSourcePlugin - Provides image/video/camera source nodes
 ******************************************************************************/
class ImageSourcePlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    ImageSourcePlugin() = default;
    ~ImageSourcePlugin() override = default;

    // Plugin identification
    QString pluginId() const override
    {
        return "imagesource";
    }

    QString pluginName() const override
    {
        return "Image Source Plugin";
    }

    QString pluginVersion() const override
    {
        return "1.0.0";
    }

    QString pluginDescription() const override
    {
        return "Provides image, video, and camera source nodes for loading visual data";
    }

    QString pluginAuthor() const override
    {
        return "VisionBox Team";
    }

    QStringList categories() const override
    {
        return QStringList() << "Sources";
    }

    // Node model creation
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>>
        createNodeModels() const override;

    // Lifecycle
    void initialize() override;
    void cleanup() override;
};

} // namespace VisionBox

#endif // VISIONBOX_IMAGESOURCEPLUGIN_H
