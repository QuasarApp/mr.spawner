//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef INTERFACES_ICONTEXTSTORAGE_H
#define INTERFACES_ICONTEXTSTORAGE_H

#include "Interfaces/ContextMessage.h"
#include "Interfaces/ContextSession.h"

#include <QSharedPointer>
#include <QString>
#include <QStringList>

namespace MrSpawner {

/**
 * @brief The IContextStorage interface
 *
 * The manager of the agent context storage. The context storage keeps
 * the conversation contexts of the agent sessions (see the sessions
 * system of Cline): every session stores its own ordered message
 * history and the agent switches between the sessions while it works.
 *
 * The Core module provides the implementation of this interface.
 *
 * IContextStorage is a QObject, the Qt base class of the project:
 * the session and context changes are announced through the
 * sessionCreated, sessionRemoved, activeSessionChanged and
 * messageAppended signals.
 */
class IContextStorage : public QObject {
    Q_OBJECT
public:
    virtual ~IContextStorage() = default;

    /**
     * @brief createSession creates a new session with a unique id
     * @param sessionTitle the title of the session, a default title
     *                     "Session <n>" is used when the title is empty
     * @return the created session; the new session becomes the active session
     */
    virtual QSharedPointer<ContextSession> createSession(const QString &sessionTitle = QString()) = 0;

    /**
     * @brief session returns the session with the given id
     * @param sessionId the id of the session
     * @return the session or a null QSharedPointer when the id is unknown
     */
    virtual QSharedPointer<ContextSession> session(const QString &sessionId) const = 0;

    /**
     * @brief sessions the ids of all stored sessions in creation order
     * @return the list of session ids
     */
    virtual QStringList sessions() const = 0;

    /**
     * @brief removeSession removes the session with the given id
     * @param sessionId the id of the session to remove
     * @return false when no session with that id is stored
     */
    virtual bool removeSession(const QString &sessionId) = 0;

    /**
     * @brief renameSession changes the title of the session
     * @param sessionId the id of the session to rename
     * @param newTitle the new title of the session
     * @return false when the session is unknown or the title is empty
     */
    virtual bool renameSession(const QString &sessionId, const QString &newTitle) = 0;

    /**
     * @brief activeSession returns the session the agent is currently working on
     * @return the active session or a null QSharedPointer when no session is active
     */
    virtual QSharedPointer<ContextSession> activeSession() const = 0;

    /**
     * @brief activateSession makes the given session the active session
     * @param sessionId the id of the session to activate
     * @return false when the id is empty or no session with that id is stored
     */
    virtual bool activateSession(const QString &sessionId) = 0;

    /**
     * @brief appendMessage appends a message to the context of the active session
     * @param message the message to append
     * @return false when the message is empty or no session is active
     */
    virtual bool appendMessage(const ContextMessage &message) = 0;

    /**
     * @brief clearContext removes all messages of a session context
     * @param sessionId the id of the session to clear, the active
     *                  session when the id is empty
     * @return false when the session is unknown or no session is active
     */
    virtual bool clearContext(const QString &sessionId = QString()) = 0;

signals:
    /**
     * @brief sessionCreated a new session was added to the storage
     * @param sessionId the id of the created session
     */
    void sessionCreated(const QString &sessionId);

    /**
     * @brief sessionRemoved a session was removed from the storage
     * @param sessionId the id of the removed session
     */
    void sessionRemoved(const QString &sessionId);

    /**
     * @brief activeSessionChanged the active session of the agent changed
     * @param sessionId the id of the new active session, an empty
     *                  string when no session is active
     */
    void activeSessionChanged(const QString &sessionId);

    /**
     * @brief messageAppended a new message was added to a session context
     * @param sessionId the id of the session the message belongs to
     * @param message the appended message
     */
    void messageAppended(const QString &sessionId, const ContextMessage &message);

protected:
    explicit IContextStorage(QObject *parent = nullptr) : QObject(parent) {
    }
};

}

#endif // INTERFACES_ICONTEXTSTORAGE_H
