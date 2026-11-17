//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef INTERFACES_MCP_SERVER_H
#define INTERFACES_MCP_SERVER_H

#include "Interfaces/ToolRequest.h"
#include "Interfaces/ToolResult.h"

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QList>

namespace MrSpawner {

/**
 * @brief The MCPPurpose enum
 *
 * Classifies MCP servers by their primary purpose. The purpose is used
 * by the agent to select appropriate MCP servers for specific tasks.
 */
enum class MCPPurpose {
    Unknown = 0,          //!< The purpose is not specified
    Code,                 //!< Code analysis and manipulation
    Data,                 //!< Data processing and analysis
    Search,               //!< Search and information retrieval
    Database,             //!< Database access and queries
    File,                 //!< File system operations
    Web,                  //!< Web access and operations
    AI,                   //!< AI model interactions
    System,               //!< System-level operations
};

/**
 * @brief The MCPServer interface
 *
 * The base interface of all Model Context Protocol (MCP) servers that the
 * AI agent can connect to. MCP servers provide context and capabilities
 * that extend the agent's knowledge and functionality.
 *
 * MCPServer is a QObject, the Qt base class of the project: servers report
 * their availability and capabilities through signals.
 */
class MCPServer : public QObject {
    Q_OBJECT
public:
    virtual ~MCPServer() = default;

    /**
     * @brief name the unique name of the MCP server in the MCP protocol
     * @return the server name, e.g. "code_analyzer"
     */
    virtual QString name() const = 0;

    /**
     * @brief displayName a short human readable name of the MCP server
     * @return the display name, defaults to name()
     */
    virtual QString displayName() const {
        return name();
    }

    /**
     * @brief description the description of the MCP server that is shown to the AI model
     * @return the server description
     */
    virtual QString description() const = 0;

    /**
     * @brief capabilitiesDescription the JSON description of the server capabilities/resources
     * @return the JSON schema of the capabilities
     */
    virtual QString capabilitiesDescription() const = 0;

    /**
     * @brief purpose the primary purpose of the MCP server
     * @return the server purpose, defaults to MCPPurpose::Unknown
     */
    virtual MCPPurpose purpose() const {
        return MCPPurpose::Unknown;
    }

    /**
     * @brief requiresApproval true if connecting to the server needs user approval
     * @return true when the agent must ask the user before connecting
     */
    virtual bool requiresApproval() const {
        return false;
    }

    /**
     * @brief connect connects to the MCP server
     * @param connection the connection parameters for connecting to the server
     * @return the connection result with any capability information
     */
    virtual ToolResult connect(const ToolRequest &connection) = 0;

    /**
     * @brief disconnect disconnects from the MCP server
     * @return the disconnection result
     */
    virtual ToolResult disconnect() {
        return ToolResult();
    }

    /**
     * @brief isConnected true if the server is currently connected
     * @return true when the server is connected
     */
    virtual bool isConnected() const {
        return false;
    }

    /**
     * @brief availableCapabilities lists all available capabilities when connected
     * @return the list of available capabilities
     */
    virtual QStringList availableCapabilities() const;

    /**
     * @brief abort stops an in-flight connection or disconnection.
     * The default implementation does nothing; servers that establish
     * network connections override it to stop the connection process.
     * The method must be safe to call before connection started, after
     * it finished and multiple times.
     */
    virtual void abort() {
    }

signals:
    /**
     * @brief progress reports a progress message of an in-flight connection
     * @param message the progress message, e.g. the current step of connection
     */
    void progress(const QString &message);

    /**
     * @brief capabilitiesReady signals that the server capabilities are available
     * @param capabilities the list of available capabilities
     */
    void capabilitiesReady(const QStringList &capabilities);

    /**
     * @brief connectionError signals that an error occurred during connection
     * @param error the error message
     */
    void connectionError(const QString &error);

protected:
    explicit MCPServer(QObject *parent = nullptr) : QObject(parent) {
    }
};

}

#endif // INTERFACES_MCP_SERVER_H