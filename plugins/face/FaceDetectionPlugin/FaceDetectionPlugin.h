/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Face Detection Plugin Interface
 ******************************************************************************/

#ifndef FACEDETECTIONPLUGIN_H
#define FACEDETECTIONPLUGIN_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QVector>

namespace VisionBox {

class FaceDetectionPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    QString pluginId() const override { return "facedetection"; }
    QString pluginName() const override { return "Face Detection Plugin"; }
    QString pluginVersion() const override { return "1.0.0"; }
    QString pluginDescription() const override { return "Provides face detection operations (Haar cascade classifiers)"; }
    QString pluginAuthor() const override { return "VisionBox Team"; }
    QStringList categories() const override { return QStringList() << "Detection" << "Face" << "Biometrics"; }

    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> createNodeModels() const override;
};

} // namespace VisionBox

#endif // FACEDETECTIONPLUGIN_H
