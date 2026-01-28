/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Image Segmentation Plugin
 ******************************************************************************/

#ifndef VISIONBOX_SEGMENTATIONPLUGIN_H
#define VISIONBOX_SEGMENTATIONPLUGIN_H

#include "core/PluginInterface.h"
#include <QString>
#include <QStringList>

namespace VisionBox {

/*******************************************************************************
 * SegmentationPlugin - Provides image segmentation operations
 ******************************************************************************/
class SegmentationPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    SegmentationPlugin() = default;
    ~SegmentationPlugin() override = default;

    QString pluginId() const override
    {
        return "segmentation";
    }

    QString pluginName() const override
    {
        return "Segmentation Plugin";
    }

    QString pluginVersion() const override
    {
        return "1.0.0";
    }

    QString pluginDescription() const override
    {
        return "Provides image segmentation operations (K-means, Otsu, Adaptive)";
    }

    QString pluginAuthor() const override
    {
        return "VisionBox Team";
    }

    QStringList categories() const override
    {
        return QStringList() << "Segmentation";
    }

    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>>
        createNodeModels() const override;

    void initialize() override {}
    void cleanup() override {}
};

} // namespace VisionBox

#endif // VISIONBOX_SEGMENTATIONPLUGIN_H
