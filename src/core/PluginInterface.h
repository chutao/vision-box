/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Plugin Interface Definition
 ******************************************************************************/

#ifndef VISIONBOX_PLUGININTERFACE_H
#define VISIONBOX_PLUGININTERFACE_H

#include <QtGlobal>
#include <QString>
#include <QStringList>
#include <vector>
#include <memory>

// Include QtNodes headers
#include <QtNodes/NodeDelegateModel>

namespace VisionBox {

/*******************************************************************************
 * IVisionNodePlugin - Interface for VisionBox plugins
 *
 * All VisionBox plugins must implement this interface to provide
 * computer vision nodes to the framework.
 ******************************************************************************/
class IVisionNodePlugin : public QObject
{
    Q_OBJECT
public:
    virtual ~IVisionNodePlugin() = default;

    /***************************************************************************
     * Plugin Identification
     **************************************************************************/

    // Unique identifier for this plugin (e.g., "com.visionbox.source")
    virtual QString pluginId() const = 0;

    // Human-readable plugin name (e.g., "Image Source Nodes")
    virtual QString pluginName() const = 0;

    // Plugin version string (e.g., "1.0.0")
    virtual QString pluginVersion() const = 0;

    // Plugin description (optional, default returns empty string)
    virtual QString pluginDescription() const
    {
        return QString();
    }

    // Plugin author/vendor (optional, default returns empty string)
    virtual QString pluginAuthor() const
    {
        return QString();
    }

    /***************************************************************************
     * Plugin Categories
     **************************************************************************/

    // Categories this plugin provides nodes for
    // Examples: "Source", "Filter", "Feature Detection", "Display"
    virtual QStringList categories() const = 0;

    /***************************************************************************
     * Node Model Creation
     **************************************************************************/

    // Create all node models provided by this plugin
    // Returns a vector of unique pointers to NodeDelegateModel instances
    // The caller takes ownership of the returned models
    virtual std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> createNodeModels() const = 0;

    /***************************************************************************
     * Plugin Lifecycle
     **************************************************************************/

    // Called when plugin is loaded
    // Use this for initialization that requires the plugin to be fully loaded
    virtual void initialize()
    {
    }

    // Called when plugin is about to be unloaded
    // Use this for cleanup and resource deallocation
    virtual void cleanup()
    {
    }

    /***************************************************************************
     * Optional: Plugin Dependencies
     **************************************************************************/

    // Returns a list of plugin IDs that this plugin depends on
    // The framework will ensure dependencies are loaded first
    virtual QStringList pluginDependencies() const
    {
        return QStringList();
    }

    /***************************************************************************
     * Optional: Plugin Configuration
     **************************************************************************/

    // Check if plugin is properly configured
    // Return false if plugin is missing required configuration
    virtual bool isConfigured() const
    {
        return true;
    }

    // Get configuration error message if !isConfigured()
    virtual QString configurationError() const
    {
        return QString();
    }
};

} // namespace VisionBox

// Declare the interface for Qt's plugin system
// Note: This macro tells Qt that IVisionNodePlugin is an interface
Q_DECLARE_INTERFACE(VisionBox::IVisionNodePlugin, "com.visionbox.IVisionNodePlugin/1.0")

#endif // VISIONBOX_PLUGININTERFACE_H
