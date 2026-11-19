//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef PLUGINTEST_H
#define PLUGINTEST_H

#include <QtTest>

/**
 * @brief The PluginTest class
 *
 * Tests of the IPlugin and the IPluginManager interfaces of the
 * Interfaces module. The tests implement the interfaces with a small
 * in-test dummy plugin and a dummy plugin manager and check the plugin
 * contract of the Qt plugin system based interfaces.
 */
class PluginTest : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void pluginMetadata();
    void pluginData();
    void pluginInterfaceCast();
    void pluginLifecycle();
    void pluginSignals();
    void pluginManagerProtocol();
    void pluginManagerLoad();
    void pluginManagerLoadDirectory();
    void pluginManagerSignals();
};

#endif // PLUGINTEST_H
