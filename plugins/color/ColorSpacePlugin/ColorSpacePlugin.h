/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Color Space Plugin Interface
 ******************************************************************************/

#ifndef COLORSPACEPLUGIN_H
#define COLORSPACEPLUGIN_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QVector>

namespace VisionBox {

class ColorSpacePlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    QString pluginId() const override { return "colorspace"; }
    QString pluginName() const override { return "Color Space Plugin"; }
    QString pluginVersion() const override { return "1.0.0"; }
    QString pluginDescription() const override { return "Provides color space conversion operations (HSV, Lab, YCrCb, etc.)"; }
    QString pluginAuthor() const override { return "VisionBox Team"; }
    QStringList categories() const override { return QStringList() << "Color" << "Conversion" << "Transform"; }

    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> createNodeModels() const override;
};

} // namespace VisionBox

#endif // COLORSPACEPLUGIN_H
