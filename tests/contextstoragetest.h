//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef CONTEXTSTORAGETEST_H
#define CONTEXTSTORAGETEST_H

#include <QtTest>

/**
 * @brief The ContextStorageTest class
 *
 * Tests of the context storage manager of the Core module. The tests
 * check the sessions system of the agent context (session creation,
 * listing, renaming, activation and removal), the per-session message
 * context and the JSON persistence of the sessions.
 */
class ContextStorageTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void sessionCreation();
    void sessionQueries();
    void sessionRename();
    void sessionRemoval();
    void activeSession();
    void contextMessages();
    void clearContext();
    void storageSignals();
    void persistenceRoundTrip();
    void persistenceRemovalAndCorruption();
};

#endif // CONTEXTSTORAGETEST_H
