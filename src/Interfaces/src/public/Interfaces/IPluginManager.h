//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef INTERFACES_IPLUGINMANAGER_H
#define INTERFACES_IPLUGINMANAGER_H

#include "Interfaces/IPlugin.h"

#include <QSharedPointer>
#include <QString>
#include <QStringList>

namespace MrSpawner {

/**
 * @brief The IPluginManager interface
 *
 * The central manager of the Mr.Spawner plugin system. The plugin manager
 * loads the plugins of the agent from the shared libraries of the Qt
 * plugin system (QPluginLoader), registers them, controls their lifecycle
 * (IPlugin::initialize() and IPlugin::shutdown()) and announces registry
 * changes to the agent. The Core module provides the implementation of
 * this interface.
 *
 * A plugin is a shared library that implements the IPlugin interface and
 * is registered with the Qt plugin system (Q_PLUGIN_METADATA() +
 * Q_INTERFACES()). The manager loads the plugin library with a
 * QPluginLoader, casts the root component to IPlugin with
 * qobject_cast<MrSpawner::IPlugin *>(), calls initialize() and keeps the
 * plugin registered until unloadPlugin() shuts the plugin down and
 * unloads the plugin library again (QPluginLoader::unload()).
 *
 * IPluginManager is a QObject, the Qt base class of the project: registry
 * changes are announced to the agent through the pluginRegistered and
 * pluginUnregistered signals.
 */
class IPluginManager : public QObject {
    Q_OBJECT
public:
    virtual ~IPluginManager() = default;

    /**
     * @brief registerPlugin adds a plugin to the registry
     * @param plugin the plugin to register
     * @return false when a plugin with the same name is already registered
     */
    virtual bool registerPlugin(const QSharedPointer<IPlugin> &plugin) = 0;

    /**
     * @brief unregisterPlugin removes a plugin from the registry
     * @param pluginName the name of the plugin to remove
     * @return false when no plugin with that name is registered
     */
    virtual bool unregisterPlugin(const QString &pluginName) = 0;

    /**
     * @brief hasPlugin true if a plugin with the given name is registered
     * @param pluginName the name of the plugin
     * @return true when the plugin is registered
     */
    virtual bool hasPlugin(const QString &pluginName) const = 0;

    /**
     * @brief plugin returns the registered plugin
     * @param pluginName the name of the plugin
     * @return the plugin or a null QSharedPointer when the name is unknown
     */
    virtual QSharedPointer<IPlugin> plugin(const QString &pluginName) const = 0;

    /**
     * @brief plugins the names of all registered plugins
     * @return the list of plugin names
     */
    virtual QStringList plugins() const = 0;

    /**
     * @brief loadPlugin loads a plugin from a shared library
     *
     * The plugin library is loaded with the Qt plugin loader
     * (QPluginLoader), the root component is cast to IPlugin and the
     * plugin is registered. The manager calls IPlugin::initialize()
     * after the plugin was loaded. On failure (unreadable library, an
     * unknown interface, a failed initialization) the method returns
     * false and stores the reason, which can be queried with
     * pluginError().
     * @param pluginPath the path of the plugin shared library
     * @return true when the plugin was loaded and initialized
     */
    virtual bool loadPlugin(const QString &pluginPath) = 0;

    /**
     * @brief loadPlugins loads all the plugins of a directory
     *
     * Every file of the given directory is loaded with loadPlugin(). The
     * files that are not valid Mr.Spawner plugins (unknown interface,
     * unreadable library) are skipped with an error message that can be
     * queried with pluginError().
     * @param pluginDirectory the directory with the plugin shared libraries
     * @return false when the directory does not exist or is not readable
     */
    virtual bool loadPlugins(const QString &pluginDirectory) = 0;

    /**
     * @brief unloadPlugin shuts the plugin down and unloads its library
     *
     * The manager calls IPlugin::shutdown() of the plugin and unloads the
     * plugin library with QPluginLoader::unload(). The plugin is removed
     * from the registry.
     * @param pluginName the name of the plugin to unload
     * @return false when no plugin with that name is registered
     */
    virtual bool unloadPlugin(const QString &pluginName) = 0;

    /**
     * @brief pluginError the last error of a plugin
     *
     * The last load or initialization error of the plugin, e.g. the
     * reason why loadPlugin() or IPlugin::initialize() failed.
     * @param pluginName the name of the plugin
     * @return the error message, empty when no error was reported
     */
    virtual QString pluginError(const QString &pluginName) const = 0;

signals:
    /**
     * @brief pluginRegistered a new plugin was added to the registry
     * @param pluginName the name of the registered plugin
     */
    void pluginRegistered(const QString &pluginName);

    /**
     * @brief pluginUnregistered a plugin was removed from the registry
     * @param pluginName the name of the removed plugin
     */
    void pluginUnregistered(const QString &pluginName);

protected:
    explicit IPluginManager(QObject *parent = nullptr) : QObject(parent) {
    }
};

}

#endif // INTERFACES_IPLUGINMANAGER_H