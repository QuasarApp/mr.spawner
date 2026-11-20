//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef INTERFACES_CONTEXTMESSAGE_H
#define INTERFACES_CONTEXTMESSAGE_H

#include <QDateTime>
#include <QJsonObject>
#include <QString>

namespace MrSpawner {

/**
 * @brief The MessageRole enum
 *
 * The sender role of a context message. The roles follow the chat
 * protocol of the AI agents: the system message holds the agent
 * instructions, the user message is the human input, the assistant
 * message is the answer of the agent and the tool message carries the
 * result of a tool execution.
 */
enum class MessageRole {
    System = 0, //!< The system prompt / the agent instructions
    User,       //!< A message of the human user
    Assistant,  //!< A message of the AI agent
    Tool        //!< The result of a tool execution
};

/**
 * @brief The ContextMessage struct
 *
 * A single message of the agent context. The context is the ordered
 * message history that the agent keeps for one session and hands to
 * the AI model (see the sessions system of Cline).
 *
 * The content is the message text. The metadata carries the additional
 * message data as a JSON object, e.g. the tool name and the tool
 * request of a tool message or the token usage of an assistant message.
 */
struct ContextMessage {
    MessageRole role{MessageRole::User}; //!< The sender role of the message
    QString content;                     //!< The message text
    QJsonObject metadata;                //!< The additional message data (tool name, token usage, ...)
    QDateTime timestamp;                 //!< The creation time of the message

    ContextMessage() = default;

    /**
     * @brief ContextMessage creates a message with the given role, content
     * and optional metadata and timestamp
     * @param messageRole the sender role of the message
     * @param messageContent the message text
     * @param messageMetadata the additional message data, empty by default
     * @param messageTimestamp the creation time, invalid by default
     */
    ContextMessage(MessageRole messageRole,
                   const QString &messageContent,
                   const QJsonObject &messageMetadata = QJsonObject(),
                   const QDateTime &messageTimestamp = QDateTime())
        : role(messageRole), content(messageContent),
          metadata(messageMetadata), timestamp(messageTimestamp) {
    }

    /**
     * @brief isValid true if the message carries a content or metadata
     * @return true when the message is not empty
     */
    bool isValid() const {
        return !content.isEmpty() || !metadata.isEmpty();
    }
};

}

// The metatype registration is placed in the global scope and uses the
// fully qualified type name, because Q_DECLARE_METATYPE expands inside the
// Qt namespace where the unqualified name would not resolve.
Q_DECLARE_METATYPE(MrSpawner::ContextMessage)

#endif // INTERFACES_CONTEXTMESSAGE_H
