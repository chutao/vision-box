/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Histogram Plugin
 ******************************************************************************/

#ifndef VISIONBOX_HISTOGRAMPLUGIN_H
#define VISIONBOX_HISTOGRAMPLUGIN_H

#include "core/PluginInterface.h"
#include <QString>
#include <QStringList>

namespace VisionBox {

/*******************************************************************************
 * HistogramPlugin - Provides histogram-based operations
 ******************************************************************************/
class HistogramPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    HistogramPlugin() = default;
    ~HistogramPlugin() override = default;

    QString pluginId() const override
    {
        return "histogram";
    }

    QString pluginName() const override
    {
        return "Histogram Plugin";
    }

    QString pluginVersion() const override
    {
        return "1.0.0";
    }

    QString pluginDescription() const override
    {
        return "Provides histogram-based operations (equalization, CLAHE)";
    }

    QString pluginAuthor() const override
    {
        return "VisionBox Team";
    }

    QStringList categories() const override
    {
        return QStringList() << "Histogram";
    }

    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>>
        createNodeModels() const override;

    void initialize() override {}
    void cleanup() override {}
};

} // namespace VisionBox

#endif // VISIONBOX_HISTOGRAMPLUGIN_H
