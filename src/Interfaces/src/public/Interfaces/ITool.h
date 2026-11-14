//
// Copyright (C) 2020-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef INTERFACES_ITOOL_H
#define INTERFACES_ITOOL_H

#include "Interfaces/ToolRequest.h"
#include "Interfaces/ToolResult.h"

namespace MrSpawner {

/**
 * @brief The ToolCategory enum
 *
 * Classifies tools by the kind of operation they perform. The category
 * is used by the agent to select, group or restrict tools (e.g. only
 * read-only tools for an analysis request).
 */
enum class ToolCategory {
    Unknown = 0, //!< The tool does not fit into a specific category
    Read,        //!< Read-only access to files and other resources
    Write,       //!< Creates or modifies files and other state
    Execute,     //!< Runs external commands or processes
    Search,      //!< Searches and analyzes code or files
    Agent        //!< Agent-level actions (sub-agents, remote tools, ...)
};

/**
 * @brief The ITool interface
 *
 * The base interface of all tools that the AI agent can call. A tool is
 * described by its name, a description for the AI model and a JSON
 * description of its input parameters (see the tool protocol of the
 * Cline and the llama.qtcreator plugin). The Core module provides the
 * concrete tool implementations of this interface.
 */
class ITool {
public:
    virtual ~ITool() = default;

    /**
     * @brief name the unique name of the tool in the tool protocol
     * @return the tool name, e.g. "read_file"
     */
    virtual std::string name() const = 0;

    /**
     * @brief displayName a short human readable name of the tool
     * @return the display name, defaults to name()
     */
    virtual std::string displayName() const {
        return name();
    }

    /**
     * @brief description the description of the tool that is shown to the AI model
     * @return the tool description
     */
    virtual std::string description() const = 0;

    /**
     * @brief parametersDescription the JSON description of the tool input parameters
     * @return the JSON schema of the parameters
     */
    virtual std::string parametersDescription() const = 0;

    /**
     * @brief category the category of the tool operation
     * @return the tool category, defaults to ToolCategory::Unknown
     */
    virtual ToolCategory category() const {
        return ToolCategory::Unknown;
    }

    /**
     * @brief requiresApproval true if the tool call needs user approval before execution
     * @return true when the agent must ask the user before running the tool
     */
    virtual bool requiresApproval() const {
        return false;
    }

    /**
     * @brief execute runs the tool with the given request
     * @param request the tool call request with the tool name and the input parameters
     * @return the tool execution result
     */
    virtual ToolResult execute(const ToolRequest &request) = 0;

    /**
     * @brief abort stops an in-flight tool execution.
     * The default implementation does nothing; tools that run processes
     * or network requests override it to stop the work. The method must
     * be safe to call before the execution started, after it finished
     * and multiple times.
     */
    virtual void abort() {
    }
};

}

#endif // INTERFACES_ITOOL_H