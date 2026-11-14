//
// Copyright (C) 2020-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#include "coretest.h"

#include <Core/core.h>

void CoreTest::initTestCase() {
}

void CoreTest::cleanupTestCase() {
}

void CoreTest::coreInit() {
    QVERIFY(MrSpawner::init());
}

void CoreTest::coreVersion() {
    QVERIFY(!MrSpawner::version().empty());
}

// Standard stand-alone QTest entry point, see the Qt test tutorial:
// https://doc.qt.io/qt-6/qttestlib-tutorial1-example.html
QTEST_GUILESS_MAIN(CoreTest)
