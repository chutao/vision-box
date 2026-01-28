/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Image Enhancement Plugin
 ******************************************************************************/

#ifndef VISIONBOX_ENHANCEMENTPLUGIN_H
#define VISIONBOX_ENHANCEMENTPLUGIN_H

#include "core/PluginInterface.h"
#include <QString>
#include <QStringList>

namespace VisionBox {

/*******************************************************************************
 * EnhancementPlugin - Provides image enhancement operations
 ******************************************************************************/
class EnhancementPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    EnhancementPlugin() = default;
    ~EnhancementPlugin() override = default;

    QString pluginId() const override
    {
        return "enhancement";
    }

    QString pluginName() const override
    {
        return "Image Enhancement Plugin";
    }

    QString pluginVersion() const override
    {
        return "1.0.0";
    }

    QString pluginDescription() const override
    {
        return "Provides image enhancement operations (denoise, sharpen)";
    }

    QString pluginAuthor() const override
    {
        return "VisionBox Team";
    }

    QStringList categories() const override
    {
        return QStringList() << "Enhancement";
    }

    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>>
        createNodeModels() const override;

    void initialize() override {}
    void cleanup() override {}
};

} // namespace VisionBox

#endif // VISIONBOX_ENHANCEMENTPLUGIN_H
