//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#include "Core/contextstorage.h"

#include <QDir>
#include <QFile>
#include <QFileInfoList>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>

namespace MrSpawner {

namespace {

QString sessionFilePath(const QString &dataDirectory, const QString &sessionId) {
    return QDir(dataDirectory).filePath(sessionId + ".json");
}

QJsonObject sessionToJson(const ContextSession *session) {
    QJsonObject obj;
    obj["id"] = session->id();
    obj["title"] = session->title();
    obj["createdAt"] = session->createdAt().toString(Qt::ISODateWithMs);
    obj["updatedAt"] = session->updatedAt().toString(Qt::ISODateWithMs);

    QJsonArray messages;
    const QList<ContextMessage> &sessionMessages = session->messages();
    for (const ContextMessage &message : sessionMessages) {
        QJsonObject msg;
        msg["role"] = static_cast<int>(message.role);
        msg["content"] = message.content;
        msg["metadata"] = message.metadata;
        msg["timestamp"] = message.timestamp.toString(Qt::ISODateWithMs);
        messages.append(msg);
    }
    obj["messages"] = messages;
    return obj;
}

QSharedPointer<ContextSession> sessionFromJson(const QJsonObject &obj) {
    QString sessionId = obj.value("id").toString();
    if (sessionId.isEmpty()) {
        return QSharedPointer<ContextSession>();
    }

    QList<ContextMessage> messages;
    const QJsonArray messageArray = obj.value("messages").toArray();
    for (const QJsonValue &value : messageArray) {
        if (!value.isObject()) {
            continue;
        }
        QJsonObject msg = value.toObject();
        ContextMessage message;
        message.role = static_cast<MessageRole>(msg.value("role").toInt());
        message.content = msg.value("content").toString();
        message.metadata = msg.value("metadata").toObject();
        message.timestamp = QDateTime::fromString(msg.value("timestamp").toString(), Qt::ISODateWithMs);
        messages.append(message);
    }

    QString title = obj.value("title").toString();
    QDateTime createdAt = QDateTime::fromString(obj.value("createdAt").toString(), Qt::ISODateWithMs);
    QDateTime updatedAt = QDateTime::fromString(obj.value("updatedAt").toString(), Qt::ISODateWithMs);
    if (!createdAt.isValid()) {
        createdAt = QDateTime::currentDateTime();
    }
    if (!updatedAt.isValid()) {
        updatedAt = createdAt;
    }
    return QSharedPointer<ContextSession>::create(sessionId, title, createdAt, updatedAt, messages);
}

}

ContextStorage::ContextStorage(QObject *parent) : IContextStorage(parent) {
}

ContextStorage::ContextStorage(const QString &dataDirectory, QObject *parent)
    : IContextStorage(parent), m_dataDirectory(dataDirectory) {
    if (!m_dataDirectory.isEmpty()) {
        QDir().mkpath(m_dataDirectory);
        load();
    }
}

QSharedPointer<ContextSession> ContextStorage::createSession(const QString &sessionTitle) {
    m_sessionCounter++;
    QString title = sessionTitle.trimmed();
    if (title.isEmpty()) {
        title = QString("Session %1").arg(m_sessionCounter);
    }

    QSharedPointer<ContextSession> session = QSharedPointer<ContextSession>::create();
    session->setTitle(title);

    m_sessions.insert(session->id(), session);
    m_sessionOrder.append(session->id());
    trackSession(session);
    persistSession(session);
    emit sessionCreated(session->id());
    setActiveSession(session->id());
    return session;
}

QSharedPointer<ContextSession> ContextStorage::session(const QString &sessionId) const {
    return m_sessions.value(sessionId);
}

QStringList ContextStorage::sessions() const {
    return m_sessionOrder;
}

bool ContextStorage::removeSession(const QString &sessionId) {
    auto it = m_sessions.find(sessionId);
    if (it == m_sessions.end()) {
        return false;
    }
    m_sessions.erase(it);
    m_sessionOrder.removeOne(sessionId);
    if (!m_dataDirectory.isEmpty()) {
        QFile::remove(sessionFilePath(m_dataDirectory, sessionId));
    }
    if (m_activeSessionId == sessionId) {
        m_activeSessionId.clear();
        emit activeSessionChanged(QString());
    }
    emit sessionRemoved(sessionId);
    return true;
}

bool ContextStorage::renameSession(const QString &sessionId, const QString &newTitle) {
    QSharedPointer<ContextSession> session = this->session(sessionId);
    if (!session || !session->setTitle(newTitle)) {
        return false;
    }
    return true;
}

QSharedPointer<ContextSession> ContextStorage::activeSession() const {
    return session(m_activeSessionId);
}

bool ContextStorage::activateSession(const QString &sessionId) {
    if (sessionId.isEmpty() || !m_sessions.contains(sessionId)) {
        return false;
    }
    setActiveSession(sessionId);
    return true;
}

void ContextStorage::setActiveSession(const QString &sessionId) {
    if (m_activeSessionId == sessionId) {
        return;
    }
    m_activeSessionId = sessionId;
    emit activeSessionChanged(sessionId);
}

bool ContextStorage::appendMessage(const ContextMessage &message) {
    if (!message.isValid()) {
        return false;
    }
    QSharedPointer<ContextSession> session = activeSession();
    if (!session) {
        return false;
    }
    ContextMessage stored = session->appendMessage(message);
    emit messageAppended(session->id(), stored);
    return true;
}

bool ContextStorage::clearContext(const QString &sessionId) {
    QString id = sessionId.isEmpty() ? m_activeSessionId : sessionId;
    QSharedPointer<ContextSession> session = this->session(id);
    if (!session) {
        return false;
    }
    session->clearMessages();
    return true;
}

QString ContextStorage::dataDirectory() const {
    return m_dataDirectory;
}

bool ContextStorage::isPersistent() const {
    return !m_dataDirectory.isEmpty();
}

bool ContextStorage::save(const QString &sessionId) const {
    if (m_dataDirectory.isEmpty()) {
        return false;
    }
    QSharedPointer<ContextSession> session = this->session(sessionId);
    if (!session) {
        return false;
    }
    return writeSessionFile(session);
}

bool ContextStorage::saveAll() const {
    if (m_dataDirectory.isEmpty()) {
        return false;
    }
    bool success = true;
    for (const QString &sessionId : m_sessionOrder) {
        if (!save(sessionId)) {
            success = false;
        }
    }
    return success;
}

bool ContextStorage::load() {
    if (m_dataDirectory.isEmpty()) {
        return false;
    }
    QDir dir(m_dataDirectory);
    if (!dir.exists() && !dir.mkpath(m_dataDirectory)) {
        return false;
    }

    m_sessions.clear();
    m_sessionOrder.clear();
    m_activeSessionId.clear();
    m_sessionCounter = 0;

    QFileInfoList files = dir.entryInfoList({"*.json"}, QDir::Files);
    QDateTime newestUpdate;
    QString newestSessionId;
    for (const QFileInfo &file : files) {
        QFile sessionFile(file.absoluteFilePath());
        if (!sessionFile.open(QIODevice::ReadOnly)) {
            qWarning() << "ContextStorage: cannot read the session file" << file.absoluteFilePath();
            continue;
        }
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(sessionFile.readAll(), &parseError);
        sessionFile.close();
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            qWarning() << "ContextStorage: skipping the invalid session file" << file.absoluteFilePath();
            continue;
        }
        QSharedPointer<ContextSession> session = sessionFromJson(doc.object());
        if (!session) {
            qWarning() << "ContextStorage: skipping the invalid session file" << file.absoluteFilePath();
            continue;
        }
        m_sessions.insert(session->id(), session);
        m_sessionOrder.append(session->id());
        trackSession(session);
        if (session->updatedAt() > newestUpdate) {
            newestUpdate = session->updatedAt();
            newestSessionId = session->id();
        }
    }
    m_sessionCounter = m_sessions.size();
    if (!newestSessionId.isEmpty()) {
        setActiveSession(newestSessionId);
    }
    return true;
}

void ContextStorage::persistSession(const QSharedPointer<ContextSession> &session) const {
    if (!m_dataDirectory.isEmpty()) {
        writeSessionFile(session);
    }
}

void ContextStorage::trackSession(const QSharedPointer<ContextSession> &session) const {
    connect(session.data(), &ContextSession::messageAppended, this, [this, session](const ContextMessage &) {
        persistSession(session);
    });
    connect(session.data(), &ContextSession::messagesCleared, this, [this, session]() {
        persistSession(session);
    });
    connect(session.data(), &ContextSession::titleChanged, this, [this, session](const QString &) {
        persistSession(session);
    });
}

bool ContextStorage::writeSessionFile(const QSharedPointer<ContextSession> &session) const {
    if (!QDir().mkpath(m_dataDirectory)) {
        return false;
    }
    const QString filePath = sessionFilePath(m_dataDirectory, session->id());
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "ContextStorage: cannot write the session file" << filePath;
        return false;
    }
    file.write(QJsonDocument(sessionToJson(session.data())).toJson(QJsonDocument::Indented));
    return true;
}

}
