/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Feature Descriptors Plugin Interface
 ******************************************************************************/

#ifndef FEATUREDESCRIPTORSPLUGIN_H
#define FEATUREDESCRIPTORSPLUGIN_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QVector>

namespace VisionBox {

class FeatureDescriptorsPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    QString pluginId() const override { return "featured"; }
    QString pluginName() const override { return "Feature Descriptors Plugin"; }
    QString pluginVersion() const override { return "1.0.0"; }
    QString pluginDescription() const override { return "Provides feature detection and descriptor computation (ORB)"; }
    QString pluginAuthor() const override { return "VisionBox Team"; }
    QStringList categories() const override { return QStringList() << "Features" << "Detection" << "Tracking"; }

    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> createNodeModels() const override;
};

} // namespace VisionBox

#endif // FEATUREDESCRIPTORSPLUGIN_H
