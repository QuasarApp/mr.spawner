//
// Copyright (C) 2020-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef CORETEST_H
#define CORETEST_H

#include <QtTest>

/**
 * @brief The CoreTest class
 *
 * Fast unit tests of the Core module. These tests do not need any
 * deployment, they only check the base skeleton of the Core library.
 */
class CoreTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void coreInit();
    void coreVersion();
};

#endif // CORETEST_H
