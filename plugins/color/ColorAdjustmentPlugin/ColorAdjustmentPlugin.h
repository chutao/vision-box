/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Color Adjustment Plugin
 ******************************************************************************/

#ifndef VISIONBOX_COLORADJUSTMENTPLUGIN_H
#define VISIONBOX_COLORADJUSTMENTPLUGIN_H

#include "core/PluginInterface.h"
#include <QString>
#include <QStringList>

namespace VisionBox {

/*******************************************************************************
 * ColorAdjustmentPlugin - Provides color and tonal adjustment operations
 ******************************************************************************/
class ColorAdjustmentPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    ColorAdjustmentPlugin() = default;
    ~ColorAdjustmentPlugin() override = default;

    QString pluginId() const override
    {
        return "coloradjustment";
    }

    QString pluginName() const override
    {
        return "Color Adjustment Plugin";
    }

    QString pluginVersion() const override
    {
        return "1.0.0";
    }

    QString pluginDescription() const override
    {
        return "Provides color and tonal adjustments (brightness, contrast, saturation)";
    }

    QString pluginAuthor() const override
    {
        return "VisionBox Team";
    }

    QStringList categories() const override
    {
        return QStringList() << "Color";
    }

    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>>
        createNodeModels() const override;

    void initialize() override {}
    void cleanup() override {}
};

} // namespace VisionBox

#endif // VISIONBOX_COLORADJUSTMENTPLUGIN_H
