/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Advanced Morphology Plugin Interface
 ******************************************************************************/

#ifndef ADVANCEDMORPHOLOGYPLUGIN_H
#define ADVANCEDMORPHOLOGYPLUGIN_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QVector>

namespace VisionBox {

class AdvancedMorphologyPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    QString pluginId() const override { return "advancedmorphology"; }
    QString pluginName() const override { return "Advanced Morphology Plugin"; }
    QString pluginVersion() const override { return "1.0.0"; }
    QString pluginDescription() const override { return "Provides advanced morphological operations (top hat, black hat, gradient)"; }
    QString pluginAuthor() const override { return "VisionBox Team"; }
    QStringList categories() const override { return QStringList() << "Filter" << "Morphology" << "Shape"; }

    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> createNodeModels() const override;
};

} // namespace VisionBox

#endif // ADVANCEDMORPHOLOGYPLUGIN_H
