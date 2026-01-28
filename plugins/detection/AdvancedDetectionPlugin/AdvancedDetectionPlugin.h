/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Advanced Detection Plugin Header
 ******************************************************************************/

#ifndef VISIONBOX_ADVANCEDDETECTIONPLUGIN_H
#define VISIONBOX_ADVANCEDDETECTIONPLUGIN_H

#include "core/PluginInterface.h"
#include <QString>
#include <QStringList>

namespace VisionBox {

/***************************************************************************//**
 * AdvancedDetectionPlugin - Advanced detection and segmentation algorithms
 ******************************************************************************/
class AdvancedDetectionPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    AdvancedDetectionPlugin() = default;
    ~AdvancedDetectionPlugin() override = default;

    // Plugin identification
    QString pluginId() const override
    {
        return "advanceddetection";
    }

    QString pluginName() const override
    {
        return "Advanced Detection Plugin";
    }

    QString pluginVersion() const override
    {
        return "1.0.0";
    }

    QString pluginDescription() const override
    {
        return "Advanced object detection and segmentation algorithms (YOLO, GrabCut)";
    }

    QString pluginAuthor() const override
    {
        return "VisionBox Team";
    }

    QStringList categories() const override
    {
        return QStringList() << "Detection" << "Segmentation";
    }

    // Node model creation
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>>
        createNodeModels() const override;

    // Lifecycle
    void initialize() override;
    void cleanup() override;
};

} // namespace VisionBox

#endif // VISIONBOX_ADVANCEDDETECTIONPLUGIN_H
