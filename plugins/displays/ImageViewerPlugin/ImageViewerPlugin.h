/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Image Viewer Plugin
 ******************************************************************************/

#ifndef VISIONBOX_IMAGEVIEWERPLUGIN_H
#define VISIONBOX_IMAGEVIEWERPLUGIN_H

#include "core/PluginInterface.h"
#include <QString>
#include <QStringList>

namespace VisionBox {

/*******************************************************************************
 * ImageViewerPlugin - Provides image display nodes
 ******************************************************************************/
class ImageViewerPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    ImageViewerPlugin() = default;
    ~ImageViewerPlugin() override = default;

    QString pluginId() const override
    {
        return "imageviewer";
    }

    QString pluginName() const override
    {
        return "Image Viewer Plugin";
    }

    QString pluginVersion() const override
    {
        return "1.0.0";
    }

    QString pluginDescription() const override
    {
        return "Provides image visualization and display nodes";
    }

    QString pluginAuthor() const override
    {
        return "VisionBox Team";
    }

    QStringList categories() const override
    {
        return QStringList() << "Displays";
    }

    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>>
        createNodeModels() const override;

    void initialize() override {}
    void cleanup() override {}
};

} // namespace VisionBox

#endif // VISIONBOX_IMAGEVIEWERPLUGIN_H
