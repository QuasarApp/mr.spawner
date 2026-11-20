//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef CORE_CONTEXTSTORAGE_H
#define CORE_CONTEXTSTORAGE_H

#include "Core/global.h"
#include "Interfaces/IContextStorage.h"

#include <QString>

namespace MrSpawner {

/**
 * @brief The ContextStorage class
 *
 * The context storage manager of the agent. It manages the sessions of
 * the agent context (see the sessions system of Cline): every session
 * keeps its own ordered message history and the agent switches between
 * the sessions while it works.
 *
 * The storage is in-memory by default. When it is created with a data
 * directory, the sessions are additionally stored as JSON files in that
 * directory (one file per session) and are loaded again when the storage
 * is reopened.
 */
class CORE_EXPORT ContextStorage : public IContextStorage {
    Q_OBJECT
public:
    /**
     * @brief ContextStorage creates an in-memory context storage
     * @param parent the parent object, defaults to nullptr
     */
    explicit ContextStorage(QObject *parent = nullptr);

    /**
     * @brief ContextStorage creates a persistent context storage
     *
     * The sessions are stored as JSON files in the data directory and
     * are loaded from that directory when the storage is created. The
     * directory is created when it does not exist yet.
     * @param dataDirectory the directory the sessions are stored in
     * @param parent the parent object, defaults to nullptr
     */
    explicit ContextStorage(const QString &dataDirectory, QObject *parent = nullptr);

    QSharedPointer<ContextSession> createSession(const QString &sessionTitle = QString()) override;
    QSharedPointer<ContextSession> session(const QString &sessionId) const override;
    QStringList sessions() const override;
    bool removeSession(const QString &sessionId) override;
    bool renameSession(const QString &sessionId, const QString &newTitle) override;

    QSharedPointer<ContextSession> activeSession() const override;
    bool activateSession(const QString &sessionId) override;

    bool appendMessage(const ContextMessage &message) override;
    bool clearContext(const QString &sessionId = QString()) override;

    /**
     * @brief dataDirectory the directory the sessions are stored in
     * @return the data directory or an empty string for the in-memory storage
     */
    QString dataDirectory() const;

    /**
     * @brief isPersistent true if the sessions are stored on disk
     * @return true when the storage uses a data directory
     */
    bool isPersistent() const;

    /**
     * @brief save writes the session with the given id to the data directory
     * @param sessionId the id of the session to save
     * @return false when the storage is in-memory or the session is unknown
     */
    bool save(const QString &sessionId) const;

    /**
     * @brief saveAll writes all sessions to the data directory
     * @return false when the storage is in-memory or a session cannot be written
     */
    bool saveAll() const;

    /**
     * @brief load reads all sessions from the data directory
     *
     * The in-memory sessions are replaced by the sessions found in the
     * data directory. The session with the newest update time becomes
     * the active session.
     * @return false when the storage is in-memory
     */
    bool load();

private:
    void persistSession(const QSharedPointer<ContextSession> &session) const;
    void trackSession(const QSharedPointer<ContextSession> &session) const;
    void setActiveSession(const QString &sessionId);
    bool writeSessionFile(const QSharedPointer<ContextSession> &session) const;

    QHash<QString, QSharedPointer<ContextSession>> m_sessions;
    QStringList m_sessionOrder;
    QString m_activeSessionId;
    QString m_dataDirectory;
    int m_sessionCounter{0};
};

}

#endif // CORE_CONTEXTSTORAGE_H
