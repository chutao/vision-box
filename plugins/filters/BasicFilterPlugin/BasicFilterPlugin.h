/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Basic Filter Plugin
 ******************************************************************************/

#ifndef VISIONBOX_BASICFILTERPLUGIN_H
#define VISIONBOX_BASICFILTERPLUGIN_H

#include "core/PluginInterface.h"
#include <QString>
#include <QStringList>

namespace VisionBox {

/*******************************************************************************
 * BasicFilterPlugin - Provides basic image filtering nodes
 ******************************************************************************/
class BasicFilterPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    BasicFilterPlugin() = default;
    ~BasicFilterPlugin() override = default;

    QString pluginId() const override
    {
        return "basicfilter";
    }

    QString pluginName() const override
    {
        return "Basic Filter Plugin";
    }

    QString pluginVersion() const override
    {
        return "1.0.0";
    }

    QString pluginDescription() const override
    {
        return "Provides basic image filtering operations (blur, threshold, etc.)";
    }

    QString pluginAuthor() const override
    {
        return "VisionBox Team";
    }

    QStringList categories() const override
    {
        return QStringList() << "Filters";
    }

    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>>
        createNodeModels() const override;

    void initialize() override {}
    void cleanup() override {}
};

} // namespace VisionBox

#endif // VISIONBOX_BASICFILTERPLUGIN_H
