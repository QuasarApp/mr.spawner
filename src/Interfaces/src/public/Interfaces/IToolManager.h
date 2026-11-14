//
// Copyright (C) 2020-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef INTERFACES_ITOOLMANAGER_H
#define INTERFACES_ITOOLMANAGER_H

#include "Interfaces/ITool.h"

#include <memory>
#include <string>
#include <vector>

namespace MrSpawner {

/**
 * @brief The IToolManager interface
 *
 * The central registry and executor of the agent tools. Tools register
 * themselves here and the agent executes tool calls through this
 * interface. The Core module provides the implementation of this
 * interface.
 */
class IToolManager {
public:
    virtual ~IToolManager() = default;

    /**
     * @brief registerTool adds a tool to the registry
     * @param tool the tool to register
     * @return false when a tool with the same name is already registered
     */
    virtual bool registerTool(const std::shared_ptr<ITool> &tool) = 0;

    /**
     * @brief unregisterTool removes a tool from the registry
     * @param toolName the name of the tool to remove
     * @return false when no tool with that name is registered
     */
    virtual bool unregisterTool(const std::string &toolName) = 0;

    /**
     * @brief hasTool true if a tool with the given name is registered
     * @param toolName the name of the tool
     * @return true when the tool is registered
     */
    virtual bool hasTool(const std::string &toolName) const = 0;

    /**
     * @brief tool returns the registered tool
     * @param toolName the name of the tool
     * @return the tool or nullptr when the name is unknown
     */
    virtual std::shared_ptr<ITool> tool(const std::string &toolName) const = 0;

    /**
     * @brief tools the names of all registered tools
     * @return the list of tool names
     */
    virtual std::vector<std::string> tools() const = 0;

    /**
     * @brief execute executes the tool call of the given request
     * @param request the tool call request
     * @return the tool result; an Error result when the tool name is unknown
     */
    virtual ToolResult execute(const ToolRequest &request) = 0;
};

}

#endif // INTERFACES_ITOOLMANAGER_H