/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Object Detection Plugin Interface
 ******************************************************************************/

#ifndef OBJECTDETECTIONPLUGIN_H
#define OBJECTDETECTIONPLUGIN_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QVector>

namespace VisionBox {

class ObjectDetectionPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    QString pluginId() const override { return "objectdetection"; }
    QString pluginName() const override { return "Object Detection Plugin"; }
    QString pluginVersion() const override { return "1.0.0"; }
    QString pluginDescription() const override { return "Provides object detection operations (HOG pedestrian detection)"; }
    QString pluginAuthor() const override { return "VisionBox Team"; }
    QStringList categories() const override { return QStringList() << "Detection" << "Objects" << "Analysis"; }

    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> createNodeModels() const override;
};

} // namespace VisionBox

#endif // OBJECTDETECTIONPLUGIN_H
