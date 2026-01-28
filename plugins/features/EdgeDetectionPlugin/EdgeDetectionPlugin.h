/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Edge Detection Plugin
 ******************************************************************************/

#ifndef VISIONBOX_EDGEDETECTIONPLUGIN_H
#define VISIONBOX_EDGEDETECTIONPLUGIN_H

#include "core/PluginInterface.h"
#include <QString>
#include <QStringList>

namespace VisionBox {

/*******************************************************************************
 * EdgeDetectionPlugin - Provides edge detection nodes
 ******************************************************************************/
class EdgeDetectionPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    EdgeDetectionPlugin() = default;
    ~EdgeDetectionPlugin() override = default;

    QString pluginId() const override
    {
        return "edgedetection";
    }

    QString pluginName() const override
    {
        return "Edge Detection Plugin";
    }

    QString pluginVersion() const override
    {
        return "1.0.0";
    }

    QString pluginDescription() const override
    {
        return "Provides edge detection algorithms (Canny, Sobel, Laplacian)";
    }

    QString pluginAuthor() const override
    {
        return "VisionBox Team";
    }

    QStringList categories() const override
    {
        return QStringList() << "Features" << "Edge Detection";
    }

    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>>
        createNodeModels() const override;

    void initialize() override {}
    void cleanup() override {}
};

} // namespace VisionBox

#endif // VISIONBOX_EDGEDETECTIONPLUGIN_H
