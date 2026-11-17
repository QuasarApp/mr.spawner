//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef SKILLSTEST_H
#define SKILLSTEST_H

#include <QtTest>

/**
 * @brief The SkillsTest class
 *
 * Tests of the skill protocol interfaces of the Interfaces module. The
 * tests implement the interfaces with small in-test dummy classes and
 * check that the skill call protocol works as designed.
 */
class SkillsTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void skillDefaults();
    void skillManagerProtocol();
    void skillSignals();
};

#endif // SKILLSTEST_H