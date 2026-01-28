/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Watershed Segmentation Plugin Interface
 ******************************************************************************/

#ifndef WATERSHEDSEGMENTATIONPLUGIN_H
#define WATERSHEDSEGMENTATIONPLUGIN_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QVector>

namespace VisionBox {

class WatershedSegmentationPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    QString pluginId() const override { return "watershed"; }
    QString pluginName() const override { return "Watershed Segmentation Plugin"; }
    QString pluginVersion() const override { return "1.0.0"; }
    QString pluginDescription() const override { return "Provides watershed segmentation (marker-based image segmentation)"; }
    QString pluginAuthor() const override { return "VisionBox Team"; }
    QStringList categories() const override { return QStringList() << "Segmentation" << "Clustering" << "Analysis"; }

    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> createNodeModels() const override;
};

} // namespace VisionBox

#endif // WATERSHEDSEGMENTATIONPLUGIN_H
