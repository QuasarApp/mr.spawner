//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#include "contextstoragetest.h"

#include "Core/contextstorage.h"
#include "Interfaces/IContextStorage.h"

#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QSharedPointer>
#include <QThread>

using MrSpawner::ContextMessage;
using MrSpawner::ContextSession;
using MrSpawner::ContextStorage;
using MrSpawner::MessageRole;

void ContextStorageTest::initTestCase() {
}

void ContextStorageTest::cleanupTestCase() {
}

void ContextStorageTest::sessionCreation() {
    ContextStorage storage;

    QSharedPointer<ContextSession> first = storage.createSession("first");
    QVERIFY(first);
    QVERIFY(!first->id().isEmpty());
    QCOMPARE(first->title(), QString("first"));
    QVERIFY(first->createdAt().isValid());
    QVERIFY(first->updatedAt().isValid());
    QCOMPARE(first->messageCount(), 0);

    // A second session gets a unique id
    QSharedPointer<ContextSession> second = storage.createSession("second");
    QVERIFY(second);
    QVERIFY(first->id() != second->id());

    // Empty titles get a generated default title
    QSharedPointer<ContextSession> third = storage.createSession();
    QVERIFY(third);
    QCOMPARE(third->title(), QString("Session 3"));

    // All sessions are listed in creation order
    QStringList ids = storage.sessions();
    QCOMPARE(ids.size(), 3);
    QCOMPARE(ids.at(0), first->id());
    QCOMPARE(ids.at(1), second->id());
    QCOMPARE(ids.at(2), third->id());

    // The in-memory storage has no data directory
    QVERIFY(!storage.isPersistent());
    QVERIFY(storage.dataDirectory().isEmpty());
    QVERIFY(!storage.save(first->id()));
    QVERIFY(!storage.saveAll());
}

void ContextStorageTest::sessionQueries() {
    ContextStorage storage;
    QSharedPointer<ContextSession> session = storage.createSession("query");

    QCOMPARE(storage.session(session->id()).data(), session.data());
    QVERIFY(!storage.session("unknown-session"));
    QVERIFY(!storage.session(""));
}

void ContextStorageTest::sessionRename() {
    ContextStorage storage;
    QSharedPointer<ContextSession> session = storage.createSession("old title");

    QVERIFY(storage.renameSession(session->id(), "new title"));
    QCOMPARE(session->title(), QString("new title"));

    QVERIFY(!storage.renameSession(session->id(), ""));
    QVERIFY(!storage.renameSession(session->id(), "   "));
    QCOMPARE(session->title(), QString("new title"));
    QVERIFY(!storage.renameSession("unknown-session", "new title"));
}

void ContextStorageTest::sessionRemoval() {
    ContextStorage storage;
    QSharedPointer<ContextSession> first = storage.createSession("first");
    QSharedPointer<ContextSession> second = storage.createSession("second");

    QVERIFY(storage.removeSession(first->id()));
    QVERIFY(!storage.session(first->id()));
    QCOMPARE(storage.sessions(), QStringList{second->id()});
    QVERIFY(!storage.removeSession(first->id()));
    QVERIFY(!storage.removeSession("unknown-session"));
}

void ContextStorageTest::activeSession() {
    ContextStorage storage;

    // No session is active on a fresh storage
    QVERIFY(!storage.activeSession());
    QVERIFY(!storage.activateSession(""));
    QVERIFY(!storage.activateSession("unknown-session"));

    // The created session becomes the active session
    QSharedPointer<ContextSession> first = storage.createSession("first");
    QVERIFY(storage.activeSession());
    QCOMPARE(storage.activeSession()->id(), first->id());

    // A newer session replaces the active session
    QSharedPointer<ContextSession> second = storage.createSession("second");
    QCOMPARE(storage.activeSession()->id(), second->id());

    // The agent can switch back to an older session
    QVERIFY(storage.activateSession(first->id()));
    QCOMPARE(storage.activeSession()->id(), first->id());

    // Removing the active session deactivates the storage
    QVERIFY(storage.removeSession(first->id()));
    QVERIFY(!storage.activeSession());
}

void ContextStorageTest::contextMessages() {
    ContextStorage storage;

    // Nothing can be appended when no session is active
    QVERIFY(!storage.appendMessage(ContextMessage(MessageRole::User, "orphan")));

    QSharedPointer<ContextSession> session = storage.createSession("chat");

    ContextMessage user(MessageRole::User, "hello");
    QVERIFY(storage.appendMessage(user));
    QCOMPARE(session->messageCount(), 1);
    QCOMPARE(session->message(0).role, MessageRole::User);
    QCOMPARE(session->message(0).content, QString("hello"));
    QVERIFY(session->message(0).timestamp.isValid());

    // Empty messages are rejected
    QVERIFY(!storage.appendMessage(ContextMessage()));
    QVERIFY(!storage.appendMessage(ContextMessage(MessageRole::User, "")));

    // Tool messages carry their tool data in the metadata
    QJsonObject metadata;
    metadata["tool"] = "read_file";
    ContextMessage tool(MessageRole::Tool, "file content", metadata);
    QVERIFY(storage.appendMessage(tool));
    QCOMPARE(session->messageCount(), 2);
    QCOMPARE(session->message(1).role, MessageRole::Tool);
    QCOMPARE(session->message(1).metadata["tool"].toString(), QString("read_file"));

    // Out-of-range message access returns an invalid message
    QVERIFY(!session->message(-1).isValid());
    QVERIFY(!session->message(session->messageCount()).isValid());

    // The session update time follows the context changes
    QVERIFY(session->updatedAt() >= session->createdAt());

    // The context can be cleared on the session itself
    QVERIFY(session->clearMessages());
    QCOMPARE(session->messageCount(), 0);
    QVERIFY(!session->clearMessages());
}

void ContextStorageTest::clearContext() {
    ContextStorage storage;
    QSharedPointer<ContextSession> first = storage.createSession("first");
    QSharedPointer<ContextSession> second = storage.createSession("second");

    storage.activateSession(first->id());
    QVERIFY(storage.appendMessage(ContextMessage(MessageRole::User, "first context")));
    storage.activateSession(second->id());
    QVERIFY(storage.appendMessage(ContextMessage(MessageRole::User, "second context")));

    // The empty id clears the context of the active session
    QVERIFY(storage.clearContext());
    QCOMPARE(second->messageCount(), 0);
    QCOMPARE(first->messageCount(), 1);

    QVERIFY(storage.clearContext(first->id()));
    QCOMPARE(first->messageCount(), 0);
    QVERIFY(!storage.clearContext("unknown-session"));
}

void ContextStorageTest::storageSignals() {
    ContextStorage storage;

    QSignalSpy createdSpy(&storage, &MrSpawner::IContextStorage::sessionCreated);
    QSignalSpy removedSpy(&storage, &MrSpawner::IContextStorage::sessionRemoved);
    QSignalSpy activeSpy(&storage, &MrSpawner::IContextStorage::activeSessionChanged);
    QSignalSpy messageSpy(&storage, &MrSpawner::IContextStorage::messageAppended);
    QCOMPARE(createdSpy.count(), 0);

    QSharedPointer<ContextSession> session = storage.createSession("signals");
    QCOMPARE(createdSpy.count(), 1);
    QCOMPARE(createdSpy.at(0).at(0).toString(), session->id());
    QCOMPARE(activeSpy.count(), 1);
    QCOMPARE(activeSpy.at(0).at(0).toString(), session->id());

    QVERIFY(storage.appendMessage(ContextMessage(MessageRole::Assistant, "answer")));
    QCOMPARE(messageSpy.count(), 1);
    QCOMPARE(messageSpy.at(0).at(0).toString(), session->id());
    QCOMPARE(messageSpy.at(0).at(1).value<ContextMessage>().content, QString("answer"));

    QVERIFY(storage.removeSession(session->id()));
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(0).toString(), session->id());
    QCOMPARE(activeSpy.count(), 2);
    QCOMPARE(activeSpy.at(1).at(0).toString(), QString());
}

void ContextStorageTest::persistenceRoundTrip() {
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString dataDir = tmpDir.filePath("context");

    ContextStorage storage(dataDir);
    QVERIFY(storage.isPersistent());
    QCOMPARE(storage.dataDirectory(), dataDir);
    QVERIFY(storage.load());

    QSharedPointer<ContextSession> session = storage.createSession("persisted");
    QJsonObject metadata;
    metadata["tool"] = "write_file";
    QVERIFY(storage.appendMessage(ContextMessage(MessageRole::User, "hello world")));
    QVERIFY(storage.appendMessage(ContextMessage(MessageRole::Tool, "written", metadata)));

    // The session is stored as a JSON file in the data directory
    QVERIFY(QFile::exists(QDir(dataDir).filePath(session->id() + ".json")));

    // A second storage instance loads the stored sessions
    ContextStorage reopened(dataDir);
    QCOMPARE(reopened.sessions(), storage.sessions());
    QVERIFY(reopened.saveAll());

    QSharedPointer<ContextSession> loaded = reopened.session(session->id());
    QVERIFY(loaded);
    QCOMPARE(loaded->title(), QString("persisted"));
    QCOMPARE(loaded->messageCount(), 2);
    QCOMPARE(loaded->message(0).role, MessageRole::User);
    QCOMPARE(loaded->message(0).content, QString("hello world"));
    QCOMPARE(loaded->message(1).role, MessageRole::Tool);
    QCOMPARE(loaded->message(1).metadata["tool"].toString(), QString("write_file"));
    QVERIFY(loaded->message(0).timestamp.isValid());

    // The session with the newest update time becomes active
    QSharedPointer<ContextSession> older = reopened.createSession("older");
    QThread::msleep(5);
    loaded->appendMessage(ContextMessage(MessageRole::Assistant, "again"));
    QVERIFY(reopened.activateSession(older->id()));
    ContextStorage reread(dataDir);
    QCOMPARE(reread.activeSession()->id(), session->id());
}

void ContextStorageTest::persistenceRemovalAndCorruption() {
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString dataDir = tmpDir.filePath("context");

    ContextStorage storage(dataDir);
    QSharedPointer<ContextSession> session = storage.createSession("doomed");
    QVERIFY(QFile::exists(QDir(dataDir).filePath(session->id() + ".json")));

    QVERIFY(storage.removeSession(session->id()));
    QVERIFY(!QFile::exists(QDir(dataDir).filePath(session->id() + ".json")));

    // Corrupted session files are skipped without aborting the load
    QFile corrupt(QDir(dataDir).filePath("corrupted.json"));
    QVERIFY(corrupt.open(QIODevice::WriteOnly));
    corrupt.write("this is not a json session");
    corrupt.close();

    QJsonObject valid;
    valid["id"] = "valid-session";
    valid["title"] = "valid";
    QFile validFile(QDir(dataDir).filePath("valid.json"));
    QVERIFY(validFile.open(QIODevice::WriteOnly));
    validFile.write(QJsonDocument(valid).toJson());
    validFile.close();

    ContextStorage reopened(dataDir);
    QCOMPARE(reopened.sessions(), QStringList{"valid-session"});
    QCOMPARE(reopened.activeSession()->id(), QString("valid-session"));
}

// Standard stand-alone QTest entry point, see the Qt test tutorial:
// https://doc.qt.io/qt-6/qttestlib-tutorial1-example.html
QTEST_GUILESS_MAIN(ContextStorageTest)
