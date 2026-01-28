/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Optical Flow Plugin Interface
 ******************************************************************************/

#ifndef OPTICALFLOWPLUGIN_H
#define OPTICALFLOWPLUGIN_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QVector>

namespace VisionBox {

class OpticalFlowPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    QString pluginId() const override { return "opticalflow"; }
    QString pluginName() const override { return "Optical Flow Plugin"; }
    QString pluginVersion() const override { return "1.0.0"; }
    QString pluginDescription() const override { return "Provides optical flow operations (motion tracking)"; }
    QString pluginAuthor() const override { return "VisionBox Team"; }
    QStringList categories() const override { return QStringList() << "Video" << "Tracking" << "Features"; }

    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> createNodeModels() const override;
};

} // namespace VisionBox

#endif // OPTICALFLOWPLUGIN_H
