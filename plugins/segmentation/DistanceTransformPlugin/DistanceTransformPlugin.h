/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Distance Transform Plugin Interface
 ******************************************************************************/

#ifndef DISTANCETRANSFORMPLUGIN_H
#define DISTANCETRANSFORMPLUGIN_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QVector>

namespace VisionBox {

class DistanceTransformPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    QString pluginId() const override { return "distancetransform"; }
    QString pluginName() const override { return "Distance Transform Plugin"; }
    QString pluginVersion() const override { return "1.0.0"; }
    QString pluginDescription() const override { return "Provides distance transform operations for binary images"; }
    QString pluginAuthor() const override { return "VisionBox Team"; }
    QStringList categories() const override { return QStringList() << "Segmentation" << "Distance" << "Analysis"; }

    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> createNodeModels() const override;
};

} // namespace VisionBox

#endif // DISTANCETRANSFORMPLUGIN_H
