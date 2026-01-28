/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Geometric Transform Plugin
 ******************************************************************************/

#ifndef VISIONBOX_GEOMETRICTRANSFORMPLUGIN_H
#define VISIONBOX_GEOMETRICTRANSFORMPLUGIN_H

#include "core/PluginInterface.h"
#include <QString>
#include <QStringList>

namespace VisionBox {

/*******************************************************************************
 * GeometricTransformPlugin - Provides geometric transformation nodes
 ******************************************************************************/
class GeometricTransformPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    GeometricTransformPlugin() = default;
    ~GeometricTransformPlugin() override = default;

    QString pluginId() const override
    {
        return "geometrictransform";
    }

    QString pluginName() const override
    {
        return "Geometric Transform Plugin";
    }

    QString pluginVersion() const override
    {
        return "1.0.0";
    }

    QString pluginDescription() const override
    {
        return "Provides geometric transformation operations (resize, rotate, flip, affine)";
    }

    QString pluginAuthor() const override
    {
        return "VisionBox Team";
    }

    QStringList categories() const override
    {
        return QStringList() << "Transforms";
    }

    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>>
        createNodeModels() const override;

    void initialize() override {}
    void cleanup() override {}
};

} // namespace VisionBox

#endif // VISIONBOX_GEOMETRICTRANSFORMPLUGIN_H
