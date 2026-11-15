//
// Copyright (C) 2020-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//


#include "quickchecktest.h"

#include "Core/QuickCheck.h"
#include "Interfaces/ILlamaServer.h"
#include "Interfaces/INotificationChecker.h"

#include <QElapsedTimer>

namespace {

/**
 * @brief The DummyNotificationChecker class
 *
 * In-test dummy of the INotificationChecker interface with call counters.
 */
class DummyNotificationChecker : public MrSpawner::INotificationChecker
{
public:
    bool unread = true;
    mutable int hasUnreadCalls = 0;
    int markAllAsReadCalls = 0;

    bool hasUnreadNotifications() const override {
        ++hasUnreadCalls;
        return unread;
    }

    bool markAllAsRead() override {
        ++markAllAsReadCalls;
        return true;
    }
};

/**
 * @brief The DummyLlamaServer class
 *
 * In-test dummy of the ILlamaServer interface with a stop call counter.
 */
class DummyLlamaServer : public MrSpawner::ILlamaServer
{
public:
    int stopCalls = 0;

    bool start() override { return true; }

    bool stop() override {
        ++stopCalls;
        return true;
    }

    bool isRunning() const override { return stopCalls == 0; }
};

}

void QuickCheckTest::initTestCase() {
}

void QuickCheckTest::cleanupTestCase() {
}

void QuickCheckTest::continueWorkWhenUnreadNotifications() {
    DummyNotificationChecker checker;
    checker.unread = true;
    DummyLlamaServer llama;

    MrSpawner::QuickCheck quickCheck(&checker, &llama, std::chrono::seconds(300));

    QElapsedTimer timer;
    timer.start();
    const auto result = quickCheck.run();
    const auto elapsed = timer.elapsed();

    QCOMPARE(result, MrSpawner::QuickCheckResult::ContinueWork);
    QCOMPARE(checker.hasUnreadCalls, 1);
    QCOMPARE(llama.stopCalls, 0);  // the llama server must stay running
    QVERIFY(elapsed < 250);        // no sleep on the continue path
}

void QuickCheckTest::markAllNotificationsAsRead() {
    DummyNotificationChecker checker;
    checker.unread = true;
    DummyLlamaServer llama;

    MrSpawner::QuickCheck quickCheck(&checker, &llama, std::chrono::seconds(300));

    QCOMPARE(quickCheck.run(), MrSpawner::QuickCheckResult::ContinueWork);
    QCOMPARE(checker.markAllAsReadCalls, 1);  // ALL notifications are marked as read
}

void QuickCheckTest::sleepAndShutDownLlamaServerWhenNoUnreadNotifications() {
    DummyNotificationChecker checker;
    checker.unread = false;
    DummyLlamaServer llama;

    MrSpawner::QuickCheck quickCheck(&checker, &llama, std::chrono::seconds(1));
    QCOMPARE(quickCheck.sleepTimeout().count(), 1);

    QElapsedTimer timer;
    timer.start();
    const auto result = quickCheck.run();
    const auto elapsed = timer.elapsed();

    QCOMPARE(result, MrSpawner::QuickCheckResult::Sleep);
    QCOMPARE(checker.markAllAsReadCalls, 0);  // nothing to mark as read
    QCOMPARE(llama.stopCalls, 1);             // the llama server is shut down
    QVERIFY(elapsed >= 1000);                 // the agent slept for the timeout
}

// Standard stand-alone QTest entry point, see the Qt test tutorial:
// https://doc.qt.io/qt-6/qttestlib-tutorial1-example.html
QTEST_GUILESS_MAIN(QuickCheckTest)
