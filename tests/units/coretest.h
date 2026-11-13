//
// Copyright (C) 2020-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef CORE_TEST_H
#define CORE_TEST_H

#include <testcore/itest.h>
#include "testcore/testutils.h"

#include <QtTest>

class CoreTest: public testcore::ITest, protected testcore::TestUtils
{
public:
    CoreTest();
    ~CoreTest();

    void test();

};

#endif // CORE_TEST_H
