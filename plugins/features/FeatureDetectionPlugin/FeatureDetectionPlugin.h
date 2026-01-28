/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Feature Detection Plugin
 ******************************************************************************/

#ifndef VISIONBOX_FEATUREDETECTIONPLUGIN_H
#define VISIONBOX_FEATUREDETECTIONPLUGIN_H

#include "core/PluginInterface.h"
#include <QString>
#include <QStringList>

namespace VisionBox {

/*******************************************************************************
 * FeatureDetectionPlugin - Provides feature detection operations
 ******************************************************************************/
class FeatureDetectionPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    FeatureDetectionPlugin() = default;
    ~FeatureDetectionPlugin() override = default;

    QString pluginId() const override
    {
        return "featuredetection";
    }

    QString pluginName() const override
    {
        return "Feature Detection Plugin";
    }

    QString pluginVersion() const override
    {
        return "1.0.0";
    }

    QString pluginDescription() const override
    {
        return "Provides feature detection operations (Harris corners, Shi-Tomasi)";
    }

    QString pluginAuthor() const override
    {
        return "VisionBox Team";
    }

    QStringList categories() const override
    {
        return QStringList() << "Features";
    }

    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>>
        createNodeModels() const override;

    void initialize() override {}
    void cleanup() override {}
};

} // namespace VisionBox

#endif // VISIONBOX_FEATUREDETECTIONPLUGIN_H
