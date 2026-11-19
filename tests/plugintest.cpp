//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#include "plugintest.h"

#include "Interfaces/IPlugin.h"
#include "Interfaces/IPluginManager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonObject>
#include <QJsonValue>
#include <QMetaType>
#include <QObject>
#include <QSignalSpy>
#include <QSharedPointer>
#include <QString>
#include <QTemporaryDir>
#include <QVariant>

namespace {

/**
 * @brief The TestPlugin class
 *
 * A minimal IPlugin implementation to verify the plugin contract. It is
 * registered with the Qt plugin system like a real plugin: Q_OBJECT,
 * Q_INTERFACES and the plugin metadata.
 */
class TestPlugin : public MrSpawner::IPlugin {
    Q_OBJECT
    Q_INTERFACES(MrSpawner::IPlugin)
public:
    explicit TestPlugin(const QString &name = QStringLiteral("test-plugin"), QObject *parent = nullptr)
        : MrSpawner::IPlugin(parent), m_name(name) {
    }

    QString name() const override {
        return m_name;
    }

    QString description() const override {
        return QStringLiteral("A dummy plugin to test the IPlugin interface");
    }

    QString version() const override {
        return QStringLiteral("1.0.0");
    }

    QJsonObject data() const override {
        QJsonObject data;
        data.insert(QStringLiteral("key"), QStringLiteral("value"));
        return data;
    }

    bool initialize() override {
        if (m_failInitialize) {
            setError(QStringLiteral("the test plugin refuses to initialize"));
            return false;
        }
        setInitialized(true);
        emit progress(QStringLiteral("initializing"));
        return true;
    }

    void shutdown() override {
        setInitialized(false);
    }

    void failInitialize(bool fail) {
        m_failInitialize = fail;
    }

private:
    bool m_failInitialize = false;
    QString m_name;
};

/**
 * @brief The DummyPluginManager class
 *
 * A minimal in-memory IPluginManager implementation to verify the plugin
 * manager protocol contract. The loadPlugin() and unloadPlugin() methods
 * simulate the QPluginLoader based plugin loading of the Qt plugin
 * system: a plugin file is a marker file and the file name without the
 * suffix is the name of the loaded plugin, like the Qt plugin loader
 * derives it from the shared library.
 */
class DummyPluginManager : public MrSpawner::IPluginManager {
public:
    explicit DummyPluginManager(QObject *parent = nullptr) : MrSpawner::IPluginManager(parent) {
    }

    bool registerPlugin(const QSharedPointer<MrSpawner::IPlugin> &plugin) override {
        if (m_plugins.contains(plugin->name())) {
            return false;
        }
        m_plugins.insert(plugin->name(), plugin);
        emit pluginRegistered(plugin->name());
        return true;
    }

    bool unregisterPlugin(const QString &pluginName) override {
        if (m_plugins.remove(pluginName) == 0) {
            return false;
        }
        emit pluginUnregistered(pluginName);
        return true;
    }

    bool hasPlugin(const QString &pluginName) const override {
        return m_plugins.contains(pluginName);
    }

    QSharedPointer<MrSpawner::IPlugin> plugin(const QString &pluginName) const override {
        return m_plugins.value(pluginName);
    }

    QStringList plugins() const override {
        return m_plugins.keys();
    }

    bool loadPlugin(const QString &pluginPath) override {
        const QFileInfo info(pluginPath);
        if (!info.exists() || !info.isFile()) {
            // The error is stored under the plugin name that the file
            // would have, so it can be queried with pluginError().
            m_errors.insert(info.completeBaseName(), QStringLiteral("the plugin file does not exist: ") + pluginPath);
            return false;
        }
        const QString name = info.completeBaseName();
        if (m_plugins.contains(name)) {
            m_errors.insert(name, QStringLiteral("a plugin with the name is already registered"));
            return false;
        }
        auto plugin = QSharedPointer<MrSpawner::IPlugin>(new TestPlugin(name));
        if (!plugin->initialize()) {
            m_errors.insert(name, plugin->error());
            return false;
        }
        m_plugins.insert(name, plugin);
        emit pluginRegistered(name);
        return true;
    }

    bool loadPlugins(const QString &pluginDirectory) override {
        QDir dir(pluginDirectory);
        if (!dir.exists() || !dir.isReadable()) {
            return false;
        }
        for (const QString &file : dir.entryList(QDir::Files)) {
            loadPlugin(dir.absoluteFilePath(file));
        }
        return true;
    }

    bool unloadPlugin(const QString &pluginName) override {
        auto it = m_plugins.constFind(pluginName);
        if (it == m_plugins.constEnd()) {
            return false;
        }
        it.value()->shutdown();
        m_plugins.erase(it);
        emit pluginUnregistered(pluginName);
        return true;
    }

    QString pluginError(const QString &pluginName) const override {
        return m_errors.value(pluginName);
    }

private:
    QHash<QString, QSharedPointer<MrSpawner::IPlugin>> m_plugins;
    QHash<QString, QString> m_errors;
};

}

void PluginTest::initTestCase() {
}

void PluginTest::cleanupTestCase() {
}

void PluginTest::pluginMetadata() {
    TestPlugin plugin;
    QCOMPARE(plugin.name(), QString("test-plugin"));
    QCOMPARE(plugin.description(), QString("A dummy plugin to test the IPlugin interface"));
    QCOMPARE(plugin.version(), QString("1.0.0"));

    // The default implementations of the optional metadata methods must
    // work: displayName defaults to name(), vendor and license are empty.
    QCOMPARE(plugin.displayName(), plugin.name());
    QVERIFY(plugin.vendor().isEmpty());
    QVERIFY(plugin.license().isEmpty());
}

void PluginTest::pluginData() {
    TestPlugin plugin;
    QVERIFY(plugin.hasData("key"));
    QCOMPARE(plugin.dataValue("key").toString(), QString("value"));
    QVERIFY(plugin.data().contains("key"));

    // Unknown keys must not be reported.
    QVERIFY(!plugin.hasData("unknown"));
    // An unknown key yields the default undefined QJsonValue variant.
    QVERIFY(plugin.dataValue("unknown").typeId() == QMetaType::QJsonValue);
    QVERIFY(plugin.dataValue("unknown").value<QJsonValue>().isUndefined());
}

void PluginTest::pluginInterfaceCast() {
    // The interface identifier of the Qt plugin system must be the
    // identifier registered with Q_DECLARE_INTERFACE.
    QCOMPARE(QString(MrSpawner::IPlugin::PLUGIN_IID), QString("MrSpawner::IPlugin"));
    QCOMPARE(QString(MrSpawner::IPlugin::PLUGIN_JSON_KEY), QString("MrSpawner.PluginData"));

    // The Qt plugin loader returns the plugin as QObject. The plugin
    // interface must be reachable through the Qt plugin system interface
    // cast.
    QObject *object = new TestPlugin;
    MrSpawner::IPlugin *plugin = qobject_cast<MrSpawner::IPlugin *>(object);
    QVERIFY(plugin != nullptr);
    QCOMPARE(plugin->name(), QString("test-plugin"));
    delete object;

    // A plain QObject is not a plugin.
    QObject plain;
    QVERIFY(qobject_cast<MrSpawner::IPlugin *>(&plain) == nullptr);
}

void PluginTest::pluginLifecycle() {
    TestPlugin plugin;
    QVERIFY(!plugin.isInitialized());
    QVERIFY(plugin.error().isEmpty());

    // The successful initialization must set the initialized state.
    QVERIFY(plugin.initialize());
    QVERIFY(plugin.isInitialized());

    // shutdown() must reset the initialized state.
    plugin.shutdown();
    QVERIFY(!plugin.isInitialized());

    // A failed initialization must report the error and stay uninitialized.
    TestPlugin failing;
    failing.failInitialize(true);
    QVERIFY(!failing.initialize());
    QVERIFY(!failing.isInitialized());
    QCOMPARE(failing.error(), QString("the test plugin refuses to initialize"));
}

void PluginTest::pluginSignals() {
    // The plugin must report its progress through the QObject signal.
    TestPlugin plugin;
    QSignalSpy progressSpy(&plugin, &MrSpawner::IPlugin::progress);
    QVERIFY(progressSpy.isValid());

    QVERIFY(plugin.initialize());
    QCOMPARE(progressSpy.count(), 1);
    QCOMPARE(progressSpy.at(0).at(0).toString(), QString("initializing"));
}

void PluginTest::pluginManagerProtocol() {
    DummyPluginManager manager;
    QVERIFY(manager.plugins().isEmpty());
    QVERIFY(!manager.hasPlugin("test-plugin"));
    QVERIFY(!manager.plugin("test-plugin"));
    // Without a load or initialization attempt no error is reported.
    QVERIFY(manager.pluginError("test-plugin").isEmpty());

    auto plugin = QSharedPointer<MrSpawner::IPlugin>(new TestPlugin);
    QVERIFY(manager.registerPlugin(plugin));
    QVERIFY(manager.hasPlugin("test-plugin"));
    QCOMPARE(manager.plugins().size(), 1);
    QCOMPARE(manager.plugin("test-plugin")->name(), QString("test-plugin"));
    // The registry must return the registered plugin object.
    QVERIFY(manager.plugin("test-plugin") == plugin);

    // A duplicate registration must be rejected.
    auto duplicate = QSharedPointer<MrSpawner::IPlugin>(new TestPlugin);
    QVERIFY(!manager.registerPlugin(duplicate));

    QVERIFY(manager.unregisterPlugin("test-plugin"));
    QVERIFY(!manager.unregisterPlugin("test-plugin"));
    QVERIFY(!manager.hasPlugin("test-plugin"));
    QVERIFY(manager.plugins().isEmpty());
}

void PluginTest::pluginManagerLoad() {
    // The plugin files of the Qt plugin system are shared libraries. For
    // the manager contract test a plain marker file is enough: the file
    // name without the suffix is the name of the plugin.
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString pluginFile = tempDir.filePath("first_plugin.so");
    {
        QFile file(pluginFile);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("dummy plugin");
        file.close();
    }

    DummyPluginManager manager;
    // A successful load registers the plugin and initializes it.
    QVERIFY(manager.loadPlugin(pluginFile));
    QVERIFY(manager.hasPlugin("first_plugin"));
    QVERIFY(manager.plugin("first_plugin") != nullptr);
    QVERIFY(manager.plugin("first_plugin")->isInitialized());

    // A second load of the same plugin must be rejected with an error.
    QVERIFY(!manager.loadPlugin(pluginFile));
    QVERIFY(manager.hasPlugin("first_plugin"));
    QVERIFY(!manager.pluginError("first_plugin").isEmpty());

    // A missing plugin file must fail with an error message.
    QVERIFY(!manager.loadPlugin(tempDir.filePath("missing_plugin.so")));
    QVERIFY(!manager.hasPlugin("missing_plugin"));
    QVERIFY(!manager.pluginError("missing_plugin").isEmpty());

    // unloadPlugin() must call shutdown() and remove the plugin from the
    // registry, like the QPluginLoader::unload() of the Qt plugin system
    // unloads the plugin library.
    auto loaded = manager.plugin("first_plugin");
    QVERIFY(loaded->isInitialized());
    QVERIFY(manager.unloadPlugin("first_plugin"));
    QVERIFY(!manager.hasPlugin("first_plugin"));
    QVERIFY(loaded->isInitialized() == false);
    // An unknown plugin cannot be unloaded.
    QVERIFY(!manager.unloadPlugin("first_plugin"));
}

void PluginTest::pluginManagerLoadDirectory() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    for (const QString &name : {QString("alpha_plugin.so"), QString("beta_plugin.so")}) {
        QFile file(tempDir.filePath(name));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("dummy plugin");
        file.close();
    }

    DummyPluginManager manager;
    // Every file of the directory must be loaded and registered.
    QVERIFY(manager.loadPlugins(tempDir.path()));
    QCOMPARE(manager.plugins().size(), 2);
    QVERIFY(manager.hasPlugin("alpha_plugin"));
    QVERIFY(manager.hasPlugin("beta_plugin"));
    QVERIFY(manager.plugin("alpha_plugin")->isInitialized());
    QVERIFY(manager.plugin("beta_plugin")->isInitialized());

    // A missing directory must be reported.
    QVERIFY(!manager.loadPlugins(tempDir.filePath("no_such_dir")));
}

void PluginTest::pluginManagerSignals() {
    // The manager must announce registry changes through the QObject
    // signals.
    DummyPluginManager manager;
    QSignalSpy registeredSpy(&manager, &MrSpawner::IPluginManager::pluginRegistered);
    QSignalSpy unregisteredSpy(&manager, &MrSpawner::IPluginManager::pluginUnregistered);
    QVERIFY(registeredSpy.isValid());
    QVERIFY(unregisteredSpy.isValid());

    QVERIFY(manager.registerPlugin(QSharedPointer<MrSpawner::IPlugin>(new TestPlugin)));
    QCOMPARE(registeredSpy.count(), 1);
    QCOMPARE(registeredSpy.at(0).at(0).toString(), QString("test-plugin"));

    QVERIFY(manager.unregisterPlugin("test-plugin"));
    QCOMPARE(unregisteredSpy.count(), 1);
    QCOMPARE(unregisteredSpy.at(0).at(0).toString(), QString("test-plugin"));
}

// Standard stand-alone QTest entry point, see the Qt test tutorial:
// https://doc.qt.io/qt-6/qttestlib-tutorial1-example.html
QTEST_GUILESS_MAIN(PluginTest)

#include "plugintest.moc"
