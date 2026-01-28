/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Template Matching Plugin
 ******************************************************************************/

#ifndef VISIONBOX_TEMPLATEMATCHINGPLUGIN_H
#define VISIONBOX_TEMPLATEMATCHINGPLUGIN_H

#include "core/PluginInterface.h"
#include <QString>
#include <QStringList>

namespace VisionBox {

/*******************************************************************************
 * TemplateMatchingPlugin - Provides template matching operations
 ******************************************************************************/
class TemplateMatchingPlugin : public IVisionNodePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.visionbox.IVisionNodePlugin" FILE "metadata.json")
    Q_INTERFACES(VisionBox::IVisionNodePlugin)

public:
    TemplateMatchingPlugin() = default;
    ~TemplateMatchingPlugin() override = default;

    QString pluginId() const override
    {
        return "templatematching";
    }

    QString pluginName() const override
    {
        return "Template Matching Plugin";
    }

    QString pluginVersion() const override
    {
        return "1.0.0";
    }

    QString pluginDescription() const override
    {
        return "Provides template matching operations (pattern detection)";
    }

    QString pluginAuthor() const override
    {
        return "VisionBox Team";
    }

    QStringList categories() const override
    {
        return QStringList() << "Matching";
    }

    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>>
        createNodeModels() const override;

    void initialize() override {}
    void cleanup() override {}
};

} // namespace VisionBox

#endif // VISIONBOX_TEMPLATEMATCHINGPLUGIN_H
