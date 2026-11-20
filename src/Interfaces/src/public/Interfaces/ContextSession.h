//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef INTERFACES_CONTEXTSESSION_H
#define INTERFACES_CONTEXTSESSION_H

#include "Interfaces/ContextMessage.h"

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QString>
#include <QUuid>

namespace MrSpawner {

/**
 * @brief The ContextSession class
 *
 * A single session of the agent context storage. A session is one
 * conversation of the agent with a user (see the sessions system of
 * Cline): it has a unique id, a human readable title and the ordered
 * message history of the conversation.
 *
 * ContextSession is a QObject, the Qt base class of the project:
 * the context changes are announced through the messageAppended and
 * the messagesCleared signals.
 */
class ContextSession : public QObject {
    Q_OBJECT
public:
    /**
     * @brief ContextSession creates a new session with the given id
     * @param sessionId the unique id of the session, a new unique id
     *                   is generated when the id is empty
     * @param parent the parent object, defaults to nullptr
     */
    explicit ContextSession(const QString &sessionId = QString(), QObject *parent = nullptr)
        : QObject(parent),
          m_id(sessionId.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : sessionId),
          m_title("Session"),
          m_createdAt(QDateTime::currentDateTime()),
          m_updatedAt(QDateTime::currentDateTime()) {
    }

    /**
     * @brief ContextSession restores a session with a serialized state
     *
     * The constructor is used by the context storage implementations
     * to load a session from its stored representation.
     * @param sessionId the unique id of the session
     * @param sessionTitle the title of the session
     * @param sessionCreatedAt the creation time of the session
     * @param sessionUpdatedAt the update time of the session
     * @param sessionMessages the message history of the session
     * @param parent the parent object, defaults to nullptr
     */
    ContextSession(const QString &sessionId,
                   const QString &sessionTitle,
                   const QDateTime &sessionCreatedAt,
                   const QDateTime &sessionUpdatedAt,
                   const QList<ContextMessage> &sessionMessages,
                   QObject *parent = nullptr)
        : QObject(parent), m_id(sessionId), m_title(sessionTitle),
          m_createdAt(sessionCreatedAt), m_updatedAt(sessionUpdatedAt),
          m_messages(sessionMessages) {
    }

    QString id() const {
        return m_id;
    }

    /**
     * @brief title the human readable title of the session
     * @return the session title
     */
    QString title() const {
        return m_title;
    }

    /**
     * @brief setTitle changes the title of the session
     * @param sessionTitle the new title of the session
     * @return false when the title is empty
     */
    bool setTitle(const QString &sessionTitle) {
        QString title = sessionTitle.trimmed();
        if (title.isEmpty()) {
            return false;
        }
        if (title == m_title) {
            return true;
        }
        m_title = title;
        emit titleChanged(m_title);
        return true;
    }

    /**
     * @brief createdAt the creation time of the session
     * @return the creation time
     */
    QDateTime createdAt() const {
        return m_createdAt;
    }

    /**
     * @brief updatedAt the time when the session context was changed the last time
     * @return the update time
     */
    QDateTime updatedAt() const {
        return m_updatedAt;
    }

    /**
     * @brief messages the ordered message history of the session context
     * @return the list of context messages
     */
    const QList<ContextMessage> &messages() const {
        return m_messages;
    }

    /**
     * @brief message the message at the given position of the context
     * @param index the position of the message in the message history
     * @return the message or an invalid (empty) message when the index is out of range
     */
    ContextMessage message(int index) const {
        if (index < 0 || index >= m_messages.size()) {
            return ContextMessage();
        }
        return m_messages.at(index);
    }

    /**
     * @brief messageCount the number of messages of the session context
     * @return the message count
     */
    int messageCount() const {
        return m_messages.size();
    }

    /**
     * @brief appendMessage appends a message to the session context
     * @param message the message to append
     * @return the stored message; a message with an invalid timestamp
     *         is stored with the current time
     */
    ContextMessage appendMessage(const ContextMessage &message) {
        ContextMessage stored = message;
        if (!stored.timestamp.isValid()) {
            stored.timestamp = QDateTime::currentDateTime();
        }
        m_messages.append(stored);
        m_updatedAt = stored.timestamp;
        emit messageAppended(stored);
        return stored;
    }

    /**
     * @brief clearMessages removes all messages of the session context
     * @return true when the context contained messages
     */
    bool clearMessages() {
        if (m_messages.isEmpty()) {
            return false;
        }
        m_messages.clear();
        m_updatedAt = QDateTime::currentDateTime();
        emit messagesCleared();
        return true;
    }

signals:
    /**
     * @brief titleChanged the title of the session was changed
     * @param title the new title of the session
     */
    void titleChanged(const QString &title);

    /**
     * @brief messageAppended a new message was added to the session context
     * @param message the appended message
     */
    void messageAppended(const ContextMessage &message);

    /**
     * @brief messagesCleared all messages of the session context were removed
     */
    void messagesCleared();

private:
    QString m_id;
    QString m_title;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
    QList<ContextMessage> m_messages;
};

}

#endif // INTERFACES_CONTEXTSESSION_H
