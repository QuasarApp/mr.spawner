//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef INTERFACES_IPLUGIN_H
#define INTERFACES_IPLUGIN_H

#include <QObject>
#include <QJsonObject>
#include <QString>
#include <QVariant>

namespace MrSpawner {

/**
 * @brief The IPlugin interface
 *
 * The base interface of all Mr.Spawner plugins. The plugin system of the
 * project inherits from the Qt plugin system: a plugin is a QObject based
 * shared library that declares this interface with the Q_PLUGIN_METADATA()
 * macro and is loaded at run time by the Qt plugin loader (QPluginLoader).
 *
 * A concrete plugin implements the metadata methods and registers itself
 * with the Qt plugin system, for example:
 *
 * @code
 * class MyPlugin : public MrSpawner::IPlugin {
 *     Q_OBJECT
 *     Q_PLUGIN_METADATA(IID "MrSpawner::IPlugin" FILE "myplugin.json")
 *     Q_INTERFACES(MrSpawner::IPlugin)
 * public:
 *     explicit MyPlugin(QObject *parent = nullptr) : MrSpawner::IPlugin(parent) {}
 *     QString name() const override { return "my_plugin"; }
 *     QString description() const override { return "My plugin"; }
 *     QString version() const override { return "1.0.0"; }
 *     bool initialize() override { return true; }
 * };
 * @endcode
 *
 * The Q_OBJECT, Q_PLUGIN_METADATA() and Q_INTERFACES() macros are the
 * plugin registration of the Qt plugin system: Q_PLUGIN_METADATA() embeds
 * the plugin metadata into the plugin library and Q_INTERFACES() tells the
 * Qt plugin system that the plugin implements this interface.
 *
 * The plugin library is loaded with a QPluginLoader and the returned root
 * component is cast to this interface with
 * qobject_cast<MrSpawner::IPlugin *>(). The Q_DECLARE_INTERFACE()
 * declaration below registers the interface in the Qt plugin system, so
 * the cast works across module boundaries.
 *
 * IPlugin is a QObject, the Qt base class of the project: the agent calls
 * initialize() after a plugin has been loaded and shutdown() before the
 * plugin library is unloaded.
 */
class IPlugin : public QObject {
    Q_OBJECT
public:
    /**
     * @brief PLUGIN_IID the identifier of the plugin interface in the Qt plugin system
     *
     * The IID is the identifier that links the plugin (Q_PLUGIN_METADATA)
     * and the interface (Q_DECLARE_INTERFACE) together in the Qt plugin
     * system.
     */
    static constexpr const char *PLUGIN_IID = "MrSpawner::IPlugin";

    /**
     * @brief PLUGIN_JSON_KEY the key of the Mr.Spawner specific data in the plugin metadata
     *
     * The plugin metadata JSON file (Q_PLUGIN_METADATA FILE) is embedded in
     * the plugin library and can be queried with QPluginLoader::metaData()
     * without loading the plugin. The Mr.Spawner specific plugin data is
     * stored under this key.
     */
    static constexpr const char *PLUGIN_JSON_KEY = "MrSpawner.PluginData";

    virtual ~IPlugin() = default;

    /**
     * @brief name the unique name of the plugin
     * @return the plugin name, e.g. "web_search"
     */
    virtual QString name() const = 0;

    /**
     * @brief displayName a short human readable name of the plugin
     * @return the display name, defaults to name()
     */
    virtual QString displayName() const {
        return name();
    }

    /**
     * @brief description the description of the plugin
     * @return the plugin description
     */
    virtual QString description() const = 0;

    /**
     * @brief version the version of the plugin
     * @return the plugin version, e.g. "1.0.0"
     */
    virtual QString version() const = 0;

    /**
     * @brief vendor the vendor/author of the plugin
     * @return the plugin vendor, defaults to an empty string
     */
    virtual QString vendor() const {
        return QString();
    }

    /**
     * @brief license the license of the plugin
     * @return the plugin license, defaults to an empty string
     */
    virtual QString license() const {
        return QString();
    }

    /**
     * @brief data the Mr.Spawner specific metadata of the plugin
     *
     * The plugin specific data, for example the configuration that the
     * plugin needs at start up. It should match the data stored under
     * PLUGIN_JSON_KEY in the plugin metadata JSON file.
     * @return the plugin data, defaults to an empty object
     */
    virtual QJsonObject data() const {
        return QJsonObject();
    }

    /**
     * @brief hasData true if the plugin data contains the given key
     * @param key the key of the data
     * @return true when the plugin data contains the key
     */
    virtual bool hasData(const QString &key) const {
        return data().contains(key);
    }

    /**
     * @brief dataValue returns the plugin data value of the given key
     * @param key the key of the data
     * @return the data value or an undefined QJsonValue variant when the key is unknown
     */
    virtual QVariant dataValue(const QString &key) const {
        return data().value(key);
    }

    /**
     * @brief initialize initializes the plugin
     *
     * The agent calls this method after the plugin library has been loaded.
     * The plugin should do its start up work here, e.g. register its tools.
     * On failure the plugin must store the reason with setError() and
     * return false.
     * @return true when the plugin was initialized successfully
     */
    virtual bool initialize() = 0;

    /**
     * @brief shutdown shuts the plugin down
     *
     * The agent calls this method before the plugin library is unloaded.
     * The default implementation does nothing; plugins that allocate
     * resources or register tools override it to clean up the work of
     * initialize().
     */
    virtual void shutdown() {
    }

    /**
     * @brief isInitialized true if the plugin was initialized
     * @return true when the plugin set the initialized state
     */
    bool isInitialized() const {
        return m_initialized;
    }

    /**
     * @brief error the last error of the plugin
     * @return the error message, e.g. the reason why initialize() failed
     */
    QString error() const {
        return m_error;
    }

signals:
    /**
     * @brief progress reports a progress message of an in-flight plugin task
     * @param message the progress message, e.g. the current step of the plugin initialization
     */
    void progress(const QString &message);

protected:
    /**
     * @brief setInitialized sets the initialized state of the plugin
     * @param state the new initialized state
     */
    void setInitialized(bool state) {
        m_initialized = state;
    }

    /**
     * @brief setError sets the last error of the plugin
     * @param error the error message
     */
    void setError(const QString &error) {
        m_error = error;
    }

    explicit IPlugin(QObject *parent = nullptr) : QObject(parent) {
    }

private:
    bool m_initialized = false;
    QString m_error;
};

}

Q_DECLARE_INTERFACE(MrSpawner::IPlugin, "MrSpawner::IPlugin")

#endif // INTERFACES_IPLUGIN_H
