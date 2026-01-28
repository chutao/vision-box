/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Plugin Manager Implementation
 ******************************************************************************/

#include "PluginManager.h"
#include <QDir>
#include <QFileInfo>
#include <QPluginLoader>
#include <QDebug>
#include <QMutexLocker>
#include <QCoreApplication>

namespace VisionBox {

// Static instance pointer
static PluginManager* s_instance = nullptr;

/*******************************************************************************
 * Constructor / Destructor
 ******************************************************************************/
PluginManager::PluginManager()
{
    // Add default plugin directories
    QString appDirPath = QCoreApplication::applicationDirPath();

    // System-wide plugins directory (same directory as executable for development)
    addPluginDirectory(appDirPath + "/plugins");

    // User-local plugins directory
    #ifdef Q_OS_WIN
        addPluginDirectory(QDir::homePath() + "/AppData/Local/VisionBox/plugins");
    #else
        addPluginDirectory(QDir::homePath() + "/.local/share/VisionBox/plugins");
    #endif
}

PluginManager::~PluginManager()
{
    unloadAllPlugins();
}

/*******************************************************************************
 * Plugin Discovery and Loading
 ******************************************************************************/
bool PluginManager::loadPlugin(const QString& pluginPath)
{
    QMutexLocker locker(&m_mutex);

    QFileInfo fileInfo(pluginPath);
    if (!fileInfo.exists())
    {
        m_lastError = QString("Plugin file does not exist: %1").arg(pluginPath);
        qWarning() << m_lastError;
        return false;
    }

    // Check if it's a shared library
    if (!fileInfo.suffix().compare("dll", Qt::CaseInsensitive) &&
        !fileInfo.suffix().compare("so", Qt::CaseInsensitive) &&
        !fileInfo.suffix().compare("dylib", Qt::CaseInsensitive))
    {
        m_lastError = QString("Plugin is not a shared library: %1").arg(pluginPath);
        qWarning() << m_lastError;
        return false;
    }

    // Check if plugin is already loaded
    for (const auto& info : m_pluginInfo)
    {
        if (info.filePath == pluginPath)
        {
            m_lastError = QString("Plugin already loaded: %1").arg(pluginPath);
            qWarning() << m_lastError;
            return false;
        }
    }

    // Create plugin loader
    QPluginLoader* loader = new QPluginLoader(pluginPath, nullptr);

    if (!loader->load())
    {
        m_lastError = QString("Failed to load plugin: %1\nError: %2")
                          .arg(pluginPath)
                          .arg(loader->errorString());
        qWarning() << m_lastError;
        delete loader;
        return false;
    }

    // Get the plugin instance
    QObject* pluginObject = loader->instance();
    if (!pluginObject)
    {
        m_lastError = QString("Failed to get plugin instance: %1").arg(pluginPath);
        qWarning() << m_lastError;
        loader->unload();
        delete loader;
        return false;
    }

    // Cast to our interface
    IVisionNodePlugin* plugin = qobject_cast<IVisionNodePlugin*>(pluginObject);
    if (!plugin)
    {
        m_lastError = QString("Plugin does not implement IVisionNodePlugin: %1")
                          .arg(pluginPath);
        qWarning() << m_lastError;
        loader->unload();
        delete loader;
        return false;
    }

    // Validate plugin
    if (!validatePlugin(plugin, pluginPath))
    {
        loader->unload();
        delete loader;
        return false;
    }

    // Resolve dependencies
    if (!resolveDependencies(plugin))
    {
        m_lastError = QString("Plugin dependencies could not be resolved: %1")
                          .arg(plugin->pluginId());
        qWarning() << m_lastError;
        loader->unload();
        delete loader;
        return false;
    }

    // Initialize plugin
    if (!initializePlugin(plugin, pluginPath))
    {
        loader->unload();
        delete loader;
        return false;
    }

    // Store the loader and plugin info
    m_loaders.append(loader);

    PluginInfo info;
    info.id = plugin->pluginId();
    info.name = plugin->pluginName();
    info.version = plugin->pluginVersion();
    info.description = plugin->pluginDescription();
    info.author = plugin->pluginAuthor();
    info.filePath = pluginPath;
    info.categories = plugin->categories();
    info.isLoaded = true;

    m_pluginInfo.append(info);

    qDebug() << "Loaded plugin:" << info.name << "v" << info.version
             << "(" << info.id << ")";

    return true;
}

int PluginManager::loadPluginsFromDirectory(const QString& directory)
{
    QDir dir(directory);
    if (!dir.exists())
    {
        qWarning() << "Plugin directory does not exist:" << directory;
        return 0;
    }

    // Find all shared libraries in the directory
    QStringList filters;
    #ifdef Q_OS_WIN
        filters << "*.dll";
    #elif defined(Q_OS_MAC)
        filters << "*.dylib";
    #else
        filters << "*.so";
    #endif

    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);

    int loadedCount = 0;
    for (const QFileInfo& fileInfo : files)
    {
        if (loadPlugin(fileInfo.absoluteFilePath()))
        {
            loadedCount++;
        }
    }

    qDebug() << QString("Loaded %1 plugins from %2").arg(loadedCount).arg(directory);
    return loadedCount;
}

void PluginManager::unloadAllPlugins()
{
    QMutexLocker locker(&m_mutex);

    // Call cleanup on all plugins before unloading
    for (QPluginLoader* loader : m_loaders)
    {
        QObject* pluginObject = loader->instance();
        if (pluginObject)
        {
            IVisionNodePlugin* plugin = qobject_cast<IVisionNodePlugin*>(pluginObject);
            if (plugin)
            {
                try
                {
                    plugin->cleanup();
                }
                catch (const std::exception& e)
                {
                    qWarning() << "Exception during plugin cleanup:" << e.what();
                }
            }
        }

        loader->unload();
        delete loader;
    }

    m_loaders.clear();
    m_pluginInfo.clear();

    qDebug() << "Unloaded all plugins";
}

bool PluginManager::reloadPlugin(const QString& pluginId)
{
    // Find the plugin
    QString pluginPath;
    for (const auto& info : m_pluginInfo)
    {
        if (info.id == pluginId)
        {
            pluginPath = info.filePath;
            break;
        }
    }

    if (pluginPath.isEmpty())
    {
        m_lastError = QString("Plugin not found: %1").arg(pluginId);
        return false;
    }

    // Unload the plugin
    for (int i = 0; i < m_loaders.size(); ++i)
    {
        if (m_pluginInfo[i].id == pluginId)
        {
            IVisionNodePlugin* plugin = qobject_cast<IVisionNodePlugin*>(m_loaders[i]->instance());
            if (plugin)
            {
                plugin->cleanup();
            }
            m_loaders[i]->unload();
            delete m_loaders[i];
            m_loaders.removeAt(i);
            m_pluginInfo.removeAt(i);
            break;
        }
    }

    // Reload the plugin
    return loadPlugin(pluginPath);
}

/*******************************************************************************
 * Plugin Query
 ******************************************************************************/
QVector<PluginInfo> PluginManager::getLoadedPlugins() const
{
    QMutexLocker locker(&m_mutex);
    return m_pluginInfo;
}

PluginInfo PluginManager::getPluginInfo(const QString& pluginId) const
{
    QMutexLocker locker(&m_mutex);
    for (const auto& info : m_pluginInfo)
    {
        if (info.id == pluginId)
        {
            return info;
        }
    }
    return PluginInfo();
}

bool PluginManager::isPluginLoaded(const QString& pluginId) const
{
    QMutexLocker locker(&m_mutex);
    for (const auto& info : m_pluginInfo)
    {
        if (info.id == pluginId)
        {
            return info.isLoaded;
        }
    }
    return false;
}

QStringList PluginManager::getPluginIds() const
{
    QMutexLocker locker(&m_mutex);
    QStringList ids;
    for (const auto& info : m_pluginInfo)
    {
        ids.append(info.id);
    }
    return ids;
}

QStringList PluginManager::getAllCategories() const
{
    QMutexLocker locker(&m_mutex);
    QSet<QString> categorySet;
    for (const auto& info : m_pluginInfo)
    {
        for (const auto& category : info.categories)
        {
            categorySet.insert(category);
        }
    }
    return categorySet.values();
}

/*******************************************************************************
 * Node Model Registration
 ******************************************************************************/
std::vector<std::unique_ptr<QtNodes::NodeDelegateModel>>
PluginManager::getRegisteredNodeModels() const
{
    QMutexLocker locker(&m_mutex);

    std::vector<std::unique_ptr<QtNodes::NodeDelegateModel>> allModels;

    for (QPluginLoader* loader : m_loaders)
    {
        QObject* pluginObject = loader->instance();
        if (!pluginObject)
        {
            continue;
        }

        IVisionNodePlugin* plugin = qobject_cast<IVisionNodePlugin*>(pluginObject);
        if (!plugin)
        {
            continue;
        }

        // Get all node models from this plugin
        auto pluginModels = plugin->createNodeModels();
        for (auto& model : pluginModels)
        {
            if (model)
            {
                allModels.push_back(std::move(model));
            }
        }
    }

    return allModels;
}

QVector<QPluginLoader*> PluginManager::getLoaders() const
{
    QMutexLocker locker(&m_mutex);
    return m_loaders;
}


/*******************************************************************************
 * Plugin Directories
 ******************************************************************************/
void PluginManager::addPluginDirectory(const QString& directory)
{
    QMutexLocker locker(&m_mutex);
    if (!m_pluginDirectories.contains(directory))
    {
        m_pluginDirectories.append(directory);
        qDebug() << "Added plugin directory:" << directory;
    }
}

void PluginManager::removePluginDirectory(const QString& directory)
{
    QMutexLocker locker(&m_mutex);
    m_pluginDirectories.removeAll(directory);
}

QStringList PluginManager::getPluginDirectories() const
{
    QMutexLocker locker(&m_mutex);
    return m_pluginDirectories;
}

void PluginManager::clearPluginDirectories()
{
    QMutexLocker locker(&m_mutex);
    m_pluginDirectories.clear();
}

/*******************************************************************************
 * Singleton Access
 ******************************************************************************/
PluginManager* PluginManager::instance()
{
    if (!s_instance)
    {
        s_instance = new PluginManager();
    }
    return s_instance;
}

QString PluginManager::lastError() const
{
    return m_lastError;
}

/*******************************************************************************
 * Private Methods
 ******************************************************************************/
bool PluginManager::validatePlugin(IVisionNodePlugin* plugin, const QString& pluginPath)
{
    // Check plugin ID
    if (plugin->pluginId().isEmpty())
    {
        m_lastError = QString("Plugin has empty ID: %1").arg(pluginPath);
        qWarning() << m_lastError;
        return false;
    }

    // Check for duplicate ID
    for (const auto& info : m_pluginInfo)
    {
        if (info.id == plugin->pluginId())
        {
            m_lastError = QString("Plugin ID already in use: %1").arg(plugin->pluginId());
            qWarning() << m_lastError;
            return false;
        }
    }

    // Check plugin name
    if (plugin->pluginName().isEmpty())
    {
        m_lastError = QString("Plugin has empty name: %1").arg(pluginPath);
        qWarning() << m_lastError;
        return false;
    }

    // Check plugin is configured
    if (!plugin->isConfigured())
    {
        m_lastError = QString("Plugin is not properly configured: %1\nError: %2")
                          .arg(plugin->pluginId())
                          .arg(plugin->configurationError());
        qWarning() << m_lastError;
        return false;
    }

    return true;
}

bool PluginManager::resolveDependencies(IVisionNodePlugin* plugin)
{
    QStringList dependencies = plugin->pluginDependencies();
    for (const QString& depId : dependencies)
    {
        bool found = false;
        for (const auto& info : m_pluginInfo)
        {
            if (info.id == depId && info.isLoaded)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            m_lastError = QString("Plugin dependency not found or not loaded: %1")
                              .arg(depId);
            return false;
        }
    }
    return true;
}

bool PluginManager::initializePlugin(IVisionNodePlugin* plugin, const QString& pluginPath)
{
    try
    {
        plugin->initialize();
        qDebug() << "Initialized plugin:" << plugin->pluginId();
        return true;
    }
    catch (const std::exception& e)
    {
        m_lastError = QString("Exception during plugin initialization: %1\nError: %2")
                          .arg(pluginPath)
                          .arg(e.what());
        qWarning() << m_lastError;
        return false;
    }
    catch (...)
    {
        m_lastError = QString("Unknown exception during plugin initialization: %1")
                          .arg(pluginPath);
        qWarning() << m_lastError;
        return false;
    }
}

} // namespace VisionBox
