/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Feature Matching Plugin
 ******************************************************************************/

#ifndef VISIONBOX_FEATUREMATCHINGPLUGIN_H
#define VISIONBOX_FEATUREMATCHINGPLUGIN_H

#include "core/PluginInterface.h"
#include <QString>
#include <QStringList>

namespace VisionBox {

/*******************************************************************************
 * FeatureMatchingPlugin - Provides feature detection and matching nodes
 ******************************************************************************/
class FeatureMatchingPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    FeatureMatchingPlugin() = default;
    ~FeatureMatchingPlugin() override = default;

    // Plugin identification
    QString pluginId() const override
    {
        return "featurematching";
    }

    QString pluginName() const override
    {
        return "Feature Matching Plugin";
    }

    QString pluginVersion() const override
    {
        return "1.0.0";
    }

    QString pluginDescription() const override
    {
        return "Provides SIFT, SURF feature detection and feature matching";
    }

    QString pluginAuthor() const override
    {
        return "VisionBox Team";
    }

    QStringList categories() const override
    {
        return QStringList() << "Features";
    }

    // Node model creation
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>>
        createNodeModels() const override;

    // Lifecycle
    void initialize() override;
    void cleanup() override;
};

} // namespace VisionBox

#endif // VISIONBOX_FEATUREMATCHINGPLUGIN_H
