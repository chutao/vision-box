/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Image Pyramid Plugin Interface
 ******************************************************************************/

#ifndef IMAGEPYRAMIDPLUGIN_H
#define IMAGEPYRAMIDPLUGIN_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QVector>

namespace VisionBox {

class ImagePyramidPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    QString pluginId() const override { return "imagepyramid"; }
    QString pluginName() const override { return "Image Pyramid Plugin"; }
    QString pluginVersion() const override { return "1.0.0"; }
    QString pluginDescription() const override { return "Provides image pyramid operations (Gaussian/Laplacian pyramids)"; }
    QString pluginAuthor() const override { return "VisionBox Team"; }
    QStringList categories() const override { return QStringList() << "Processing" << "Multi-scale" << "Analysis"; }

    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> createNodeModels() const override;
};

} // namespace VisionBox

#endif // IMAGEPYRAMIDPLUGIN_H
