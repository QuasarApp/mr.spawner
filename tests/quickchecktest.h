//
// Copyright (C) 2020-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//


#ifndef QUICKCHECKTEST_H
#define QUICKCHECKTEST_H

#include <QtTest>

/**
 * @brief The QuickCheckTest class
 *
 * Tests of the quick check that runs before the start of the main work
 * cycle of the agent (issue #7). The tests use small in-test dummy
 * implementations of the INotificationChecker and ILlamaServer interfaces.
 */
class QuickCheckTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void continueWorkWhenUnreadNotifications();
    void markAllNotificationsAsRead();
    void sleepAndShutDownLlamaServerWhenNoUnreadNotifications();
};

#endif // QUICKCHECKTEST_H
