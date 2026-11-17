//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef INTERFACES_IMCPSERVERMANAGER_H
#define INTERFACES_IMCPSERVERMANAGER_H

#include "Interfaces/McpServer.h"

#include <QSharedPointer>
#include <QString>
#include <QStringList>

namespace MrSpawner {

/**
 * @brief The IMcpServerManager interface
 *
 * The central registry and executor of the MCP servers. MCP servers register
 * themselves here and the agent connects to MCP servers through this
 * interface. The Core module provides the implementation of this
 * interface.
 *
 * IMcpServerManager is a QObject, the Qt base class of the project:
 * server changes are announced to the agent through the serverConnected
 * and serverDisconnected signals.
 */
class IMcpServerManager : public QObject {
    Q_OBJECT
public:
    virtual ~IMcpServerManager() = default;

    /**
     * @brief registerServer adds an MCP server to the registry
     * @param server the server to register
     * @return false when a server with the same name is already registered
     */
    virtual bool registerServer(const QSharedPointer<MCPServer> &server) = 0;

    /**
     * @brief unregisterServer removes an MCP server from the registry
     * @param serverName the name of the server to remove
     * @return false when no server with that name is registered
     */
    virtual bool unregisterServer(const QString &serverName) = 0;

    /**
     * @brief hasServer true if an MCP server with the given name is registered
     * @param serverName the name of the server
     * @return true when the server is registered
     */
    virtual bool hasServer(const QString &serverName) const = 0;

    /**
     * @brief server returns the registered MCP server
     * @param serverName the name of the server
     * @return the server or a null QSharedPointer when the name is unknown
     */
    virtual QSharedPointer<MCPServer> server(const QString &serverName) const = 0;

    /**
     * @brief servers the names of all registered MCP servers
     * @return the list of server names
     */
    virtual QStringList servers() const = 0;

    /**
     * @brief connect connects to the MCP server with the given request
     * @param request the server connection request
     * @return the connection result
     */
    virtual ToolResult connect(const ToolRequest &request) = 0;

    /**
     * @brief disconnect disconnects from the MCP server
     * @param serverName the name of the server to disconnect
     * @return the disconnection result
     */
    virtual ToolResult disconnect(const QString &serverName) = 0;

signals:
    /**
     * @brief serverRegistered a new MCP server was added to the registry
     * @param serverName the name of the registered server
     */
    void serverRegistered(const QString &serverName);

    /**
     * @brief serverDisconnected an MCP server was removed from the registry
     * @param serverName the name of the removed server
     */
    void serverDisconnected(const QString &serverName);

protected:
    explicit IMcpServerManager(QObject *parent = nullptr) : QObject(parent) {
    }
};

}

#endif // INTERFACES_IMCPSERVERMANAGER_H