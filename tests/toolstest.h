//
// Copyright (C) 2020-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef TOOLSTEST_H
#define TOOLSTEST_H

#include <QtTest>

/**
 * @brief The ToolsTest class
 *
 * Tests of the tool protocol interfaces of the Interfaces module. The
 * tests implement the interfaces with small in-test dummy classes and
 * check that the tool call protocol works as designed.
 */
class ToolsTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void toolRequestDefaults();
    void toolResultDefaults();
    void dummyToolContract();
    void toolManagerProtocol();
};

#endif // TOOLSTEST_H