/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Image Arithmetic Plugin
 ******************************************************************************/

#ifndef VISIONBOX_IMAGEARITHMETICPLUGIN_H
#define VISIONBOX_IMAGEARITHMETICPLUGIN_H

#include "core/PluginInterface.h"
#include <QString>
#include <QStringList>

namespace VisionBox {

/*******************************************************************************
 * ImageArithmeticPlugin - Provides image arithmetic operations
 ******************************************************************************/
class ImageArithmeticPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    ImageArithmeticPlugin() = default;
    ~ImageArithmeticPlugin() override = default;

    QString pluginId() const override
    {
        return "imagearithmetic";
    }

    QString pluginName() const override
    {
        return "Image Arithmetic Plugin";
    }

    QString pluginVersion() const override
    {
        return "1.0.0";
    }

    QString pluginDescription() const override
    {
        return "Provides image arithmetic operations (add, subtract, multiply, divide, blend)";
    }

    QString pluginAuthor() const override
    {
        return "VisionBox Team";
    }

    QStringList categories() const override
    {
        return QStringList() << "Arithmetic";
    }

    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>>
        createNodeModels() const override;

    void initialize() override {}
    void cleanup() override {}
};

} // namespace VisionBox

#endif // VISIONBOX_IMAGEARITHMETICPLUGIN_H
