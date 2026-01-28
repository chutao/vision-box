/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Plugin Manager - Loads and manages VisionBox plugins
 ******************************************************************************/

#ifndef VISIONBOX_PLUGINMANAGER_H
#define VISIONBOX_PLUGINMANAGER_H

#include "PluginInterface.h"
#include <QMutex>
#include <QPluginLoader>
#include <QJsonObject>
#include <QVector>
#include <memory>

namespace VisionBox {

/*******************************************************************************
 * PluginInfo - Metadata about a loaded plugin
 ******************************************************************************/
struct PluginInfo
{
    QString id;
    QString name;
    QString version;
    QString description;
    QString author;
    QString filePath;
    QStringList categories;
    bool isLoaded;

    PluginInfo()
        : isLoaded(false)
    {
    }
};

/*******************************************************************************
 * PluginManager - Manages plugin lifecycle and registration
 *
 * The PluginManager is responsible for:
 * - Discovering plugins in specified directories
 * - Loading plugin shared libraries
 * - Managing plugin lifecycle (initialize/cleanup)
 * - Providing access to node models from loaded plugins
 ******************************************************************************/
class PluginManager
{
public:
    PluginManager();
    ~PluginManager();

    // Prevent copying
    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;

    /***************************************************************************
     * Plugin Discovery and Loading
     **************************************************************************/

    // Load a single plugin from the given file path
    // Returns true if successful, false otherwise
    bool loadPlugin(const QString& pluginPath);

    // Load all plugins from a directory
    // Returns the number of plugins successfully loaded
    int loadPluginsFromDirectory(const QString& directory);

    // Unload all loaded plugins
    void unloadAllPlugins();

    // Reload a specific plugin (unload then load)
    bool reloadPlugin(const QString& pluginId);

    /***************************************************************************
     * Plugin Query
     **************************************************************************/

    // Get information about all loaded plugins
    QVector<PluginInfo> getLoadedPlugins() const;

    // Get information about a specific plugin by ID
    PluginInfo getPluginInfo(const QString& pluginId) const;

    // Check if a plugin is loaded
    bool isPluginLoaded(const QString& pluginId) const;

    // Get all plugin IDs
    QStringList getPluginIds() const;

    // Get all unique categories from loaded plugins
    QStringList getAllCategories() const;

    /***************************************************************************
     * Node Model Registration
     **************************************************************************/

    // Get all registered node models from all plugins
    // Returns a vector of unique pointers to NodeDelegateModel instances
    // The caller takes ownership of the models
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> getRegisteredNodeModels() const;

    // Get all plugin loaders (for advanced use)
    QVector<QPluginLoader*> getLoaders() const;

    /***************************************************************************
     * Plugin Directories
     **************************************************************************/

    // Add a directory to the plugin search path
    void addPluginDirectory(const QString& directory);

    // Remove a directory from the plugin search path
    void removePluginDirectory(const QString& directory);

    // Get all plugin search directories
    QStringList getPluginDirectories() const;

    // Clear all plugin search directories
    void clearPluginDirectories();

    /***************************************************************************
     * Singleton Access
     **************************************************************************/

    // Get the global plugin manager instance
    static PluginManager* instance();

    /***************************************************************************
     * Error Handling
     **************************************************************************/

    // Get the last error message
    QString lastError() const;

private:
    // Validate a plugin before loading
    bool validatePlugin(IVisionNodePlugin* plugin, const QString& pluginPath);

    // Resolve plugin dependencies
    bool resolveDependencies(IVisionNodePlugin* plugin);

    // Initialize a plugin after loading
    bool initializePlugin(IVisionNodePlugin* plugin, const QString& pluginPath);

    mutable QMutex m_mutex;
    QVector<QPluginLoader*> m_loaders;
    QVector<PluginInfo> m_pluginInfo;
    QStringList m_pluginDirectories;
    QString m_lastError;
};

} // namespace VisionBox

#endif // VISIONBOX_PLUGINMANAGER_H
