/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Advanced Transform Plugin
 ******************************************************************************/

#ifndef VISIONBOX_ADVANCEDTRANSFORMPLUGIN_H
#define VISIONBOX_ADVANCEDTRANSFORMPLUGIN_H

#include "core/PluginInterface.h"
#include <QString>
#include <QStringList>

namespace VisionBox {

/*******************************************************************************
 * AdvancedTransformPlugin - Provides advanced geometric transformations
 ******************************************************************************/
class AdvancedTransformPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    AdvancedTransformPlugin() = default;
    ~AdvancedTransformPlugin() override = default;

    // Plugin identification
    QString pluginId() const override
    {
        return "advancedtransform";
    }

    QString pluginName() const override
    {
        return "Advanced Transform Plugin";
    }

    QString pluginVersion() const override
    {
        return "1.0.0";
    }

    QString pluginDescription() const override
    {
        return "Provides affine and perspective transformations";
    }

    QString pluginAuthor() const override
    {
        return "VisionBox Team";
    }

    QStringList categories() const override
    {
        return QStringList() << "Transforms";
    }

    // Node model creation
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>>
        createNodeModels() const override;

    // Lifecycle
    void initialize() override;
    void cleanup() override;
};

} // namespace VisionBox

#endif // VISIONBOX_ADVANCEDTRANSFORMPLUGIN_H
