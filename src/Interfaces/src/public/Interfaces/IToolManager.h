//
// Copyright (C) 2020-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef INTERFACES_ITOOLMANAGER_H
#define INTERFACES_ITOOLMANAGER_H

#include "Interfaces/ITool.h"

#include <QSharedPointer>
#include <QString>
#include <QStringList>

namespace MrSpawner {

/**
 * @brief The IToolManager interface
 *
 * The central registry and executor of the agent tools. Tools register
 * themselves here and the agent executes tool calls through this
 * interface. The Core module provides the implementation of this
 * interface.
 *
 * IToolManager is a QObject, the Qt base class of the project: registry
 * changes are announced to the agent through the toolRegistered and
 * toolUnregistered signals.
 */
class IToolManager : public QObject {
    Q_OBJECT
public:
    virtual ~IToolManager() = default;

    /**
     * @brief registerTool adds a tool to the registry
     * @param tool the tool to register
     * @return false when a tool with the same name is already registered
     */
    virtual bool registerTool(const QSharedPointer<ITool> &tool) = 0;

    /**
     * @brief unregisterTool removes a tool from the registry
     * @param toolName the name of the tool to remove
     * @return false when no tool with that name is registered
     */
    virtual bool unregisterTool(const QString &toolName) = 0;

    /**
     * @brief hasTool true if a tool with the given name is registered
     * @param toolName the name of the tool
     * @return true when the tool is registered
     */
    virtual bool hasTool(const QString &toolName) const = 0;

    /**
     * @brief tool returns the registered tool
     * @param toolName the name of the tool
     * @return the tool or a null QSharedPointer when the name is unknown
     */
    virtual QSharedPointer<ITool> tool(const QString &toolName) const = 0;

    /**
     * @brief tools the names of all registered tools
     * @return the list of tool names
     */
    virtual QStringList tools() const = 0;

    /**
     * @brief execute executes the tool call of the given request
     * @param request the tool call request
     * @return the tool result; an Error result when the tool name is unknown
     */
    virtual ToolResult execute(const ToolRequest &request) = 0;

signals:
    /**
     * @brief toolRegistered a new tool was added to the registry
     * @param toolName the name of the registered tool
     */
    void toolRegistered(const QString &toolName);

    /**
     * @brief toolUnregistered a tool was removed from the registry
     * @param toolName the name of the removed tool
     */
    void toolUnregistered(const QString &toolName);

protected:
    explicit IToolManager(QObject *parent = nullptr) : QObject(parent) {
    }
};

}

#endif // INTERFACES_ITOOLMANAGER_H
