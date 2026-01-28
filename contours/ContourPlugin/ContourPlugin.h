/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Contour Analysis Plugin
 ******************************************************************************/

#ifndef VISIONBOX_CONTOURPLUGIN_H
#define VISIONBOX_CONTOURPLUGIN_H

#include "core/PluginInterface.h"
#include <QString>
#include <QStringList>

namespace VisionBox {

/*******************************************************************************
 * ContourPlugin - Provides contour detection and analysis operations
 ******************************************************************************/
class ContourPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    ContourPlugin() = default;
    ~ContourPlugin() override = default;

    QString pluginId() const override
    {
        return "contour";
    }

    QString pluginName() const override
    {
        return "Contour Plugin";
    }

    QString pluginVersion() const override
    {
        return "1.0.0";
    }

    QString pluginDescription() const override
    {
        return "Provides contour detection and analysis operations (find, draw, bounding boxes)";
    }

    QString pluginAuthor() const override
    {
        return "VisionBox Team";
    }

    QStringList categories() const override
    {
        return QStringList() << "Contours";
    }

    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>>
        createNodeModels() const override;

    void initialize() override {}
    void cleanup() override {}
};

} // namespace VisionBox

#endif // VISIONBOX_CONTOURPLUGIN_H
