//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef INTERFACES_INETWORKPROTOCOL_H
#define INTERFACES_INETWORKPROTOCOL_H

#include "Interfaces/ToolRequest.h"
#include "Interfaces/ToolResult.h"

#include <QObject>
#include <QJsonObject>
#include <QString>
#include <QStringList>

namespace MrSpawner {

/**
 * @brief The ProtocolType enum
 *
 * The network protocols the agent can use to communicate with an AI model.
 * The type is used by the agent to select the protocol for a request
 * (e.g. the local llama.cpp server for offline work, a cloud API for
 * hosted models).
 */
enum class ProtocolType {
    Unknown = 0,  //!< The protocol type is not specified
    LlamaCpp,     //!< The llama.cpp local model server protocol (HTTP server of a local model)
    Claude,       //!< The Anthropic Claude API protocol (cloud API)
    Gemini,       //!< The Google Gemini API protocol (cloud API)
    Gpt           //!< The OpenAI GPT API protocol (cloud API)
};

/**
 * @brief The INetworkProtocol interface
 *
 * The base interface of all network protocols the agent uses to communicate
 * with an AI model. A protocol wraps one of the supported transports
 * (see ProtocolType): the local llama.cpp model server, the Anthropic Claude
 * API, the Google Gemini API or the OpenAI GPT API. The protocol is described
 * by its name, a description for the agent and its JSON configuration
 * (endpoint, API key, model, ...), and answers model queries either
 * synchronously (query) or asynchronously (send, the streamed response
 * chunks are announced through the response signal).
 *
 * The Core module provides the concrete protocol implementations of this
 * interface.
 *
 * INetworkProtocol is a QObject, the Qt base class of the project:
 * long-running requests report their progress to the agent through the
 * progress signal, streamed response chunks are delivered through the
 * response signal and connection problems through the connectionError signal.
 */
class INetworkProtocol : public QObject {
    Q_OBJECT
public:
    virtual ~INetworkProtocol() = default;

    /**
     * @brief type the network protocol type of the protocol
     * @return the protocol type, one of the ProtocolType values
     */
    virtual ProtocolType type() const = 0;

    /**
     * @brief name the unique name of the protocol in the protocol registry
     * @return the protocol name, e.g. "llama_cpp"
     */
    virtual QString name() const = 0;

    /**
     * @brief displayName a short human readable name of the protocol
     * @return the display name, defaults to name()
     */
    virtual QString displayName() const {
        return name();
    }

    /**
     * @brief description the description of the protocol that is shown to the agent
     * @return the protocol description
     */
    virtual QString description() const = 0;

    /**
     * @brief configuration the current configuration of the protocol
     * @return the JSON configuration of the protocol (endpoint, API key, model, ...)
     */
    virtual QJsonObject configuration() const {
        return QJsonObject();
    }

    /**
     * @brief configure applies a new configuration to the protocol
     * @param configuration the new JSON configuration (endpoint, API key, model, ...)
     * @return true when the configuration was accepted
     */
    virtual bool configure(const QJsonObject &configuration) {
        return false;
    }

    /**
     * @brief models the models the protocol can query
     * @return the list of available model names
     */
    virtual QStringList models() const {
        return QStringList();
    }

    /**
     * @brief activeModel the model that is used for new queries
     * @return the active model name, empty when the protocol has no model selection
     */
    virtual QString activeModel() const {
        return QString();
    }

    /**
     * @brief selectModel selects the model that is used for new queries
     * @param model the model name to select
     * @return true when the model is available and was selected
     */
    virtual bool selectModel(const QString &model) {
        return false;
    }

    /**
     * @brief connect connects the protocol to the model provider.
     * The default implementation does nothing and reports success;
     * protocols that open a network connection (e.g. the llama.cpp server)
     * override it to establish the connection.
     * @return the connection result
     */
    virtual ToolResult connect() {
        return ToolResult();
    }

    /**
     * @brief disconnect disconnects the protocol from the model provider
     * @return the disconnection result
     */
    virtual ToolResult disconnect() {
        return ToolResult();
    }

    /**
     * @brief isConnected true if the protocol is currently connected to the model provider
     * @return true when the protocol is connected
     */
    virtual bool isConnected() const {
        return false;
    }

    /**
     * @brief query sends a model query and waits for the complete answer
     * @param request the model query request; the parameters carry the query
     *        (e.g. "prompt", "system", "messages", "model")
     * @return the query result; the output is the model answer handed back
     *         to the agent, the error carries a failure message when the
     *         status is not Success
     */
    virtual ToolResult query(const ToolRequest &request) = 0;

    /**
     * @brief send sends a model query and returns immediately. The complete
     * answer is announced through the finished signal, streamed response
     * chunks through the response signal. The default implementation runs
     * query() synchronously and emits finished with its result; streaming
     * protocols override it.
     * @param request the model query request, see query()
     * @return the send result; an Error result when the query could not be
     *         started
     */
    virtual ToolResult send(const ToolRequest &request) {
        ToolResult result = query(request);
        emit finished(result);
        return result;
    }

    /**
     * @brief abort stops an in-flight model query.
     * The default implementation does nothing; protocols that run network
     * requests override it to stop the request. The method must be safe to
     * call before the query started, after it finished and multiple times.
     */
    virtual void abort() {
    }

signals:
    /**
     * @brief progress reports a progress message of an in-flight model query
     * @param message the progress message, e.g. the current step of the query
     */
    void progress(const QString &message);

    /**
     * @brief response a streamed chunk of the model answer arrived
     * @param chunk the streamed text chunk of the model answer
     */
    void response(const QString &chunk);

    /**
     * @brief finished the asynchronous model query started by send() finished
     * @param result the query result with the complete model answer
     */
    void finished(const ToolResult &result);

    /**
     * @brief connectionError an error occurred during the protocol connection
     * @param error the error message
     */
    void connectionError(const QString &error);

protected:
    explicit INetworkProtocol(QObject *parent = nullptr) : QObject(parent) {
    }
};

}

#endif // INTERFACES_INETWORKPROTOCOL_H

