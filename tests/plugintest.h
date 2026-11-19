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
 * Tests of the IPlugin interface of the Interfaces module. The tests
 * implement the interface with a small in-test dummy plugin and check
 * the plugin contract of the Qt plugin system based interface.
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
};

#endif // PLUGINTEST_H
