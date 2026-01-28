/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Video Processing Plugin Interface
 ******************************************************************************/

#ifndef VIDEOPROCESSINGPLUGIN_H
#define VIDEOPROCESSINGPLUGIN_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QVector>

namespace VisionBox {

class VideoProcessingPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    QString pluginId() const override { return "videoprocessing"; }
    QString pluginName() const override { return "Video Processing Plugin"; }
    QString pluginVersion() const override { return "1.0.0"; }
    QString pluginDescription() const override { return "Provides video processing operations (background subtraction, motion detection)"; }
    QString pluginAuthor() const override { return "VisionBox Team"; }
    QStringList categories() const override { return QStringList() << "Video" << "Processing" << "Analysis"; }

    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> createNodeModels() const override;
};

} // namespace VisionBox

#endif // VIDEOPROCESSINGPLUGIN_H
