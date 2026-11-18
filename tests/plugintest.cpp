//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#include "plugintest.h"

#include "Interfaces/IPlugin.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QMetaType>
#include <QObject>
#include <QSignalSpy>
#include <QString>
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
    explicit TestPlugin(QObject *parent = nullptr) : MrSpawner::IPlugin(parent) {
    }

    QString name() const override {
        return QStringLiteral("test-plugin");
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

// Standard stand-alone QTest entry point, see the Qt test tutorial:
// https://doc.qt.io/qt-6/qttestlib-tutorial1-example.html
QTEST_GUILESS_MAIN(PluginTest)

#include "plugintest.moc"