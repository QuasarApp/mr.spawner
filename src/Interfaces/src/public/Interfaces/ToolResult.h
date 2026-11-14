//
// Copyright (C) 2020-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef INTERFACES_TOOLRESULT_H
#define INTERFACES_TOOLRESULT_H

#include <string>

namespace MrSpawner {

/**
 * @brief The ToolStatus enum
 *
 * The outcome status of a tool execution.
 */
enum class ToolStatus {
    Success = 0, //!< The tool ran and produced a result
    Error,       //!< The tool was executed but failed
    Rejected,    //!< The tool call was rejected before execution
    Aborted      //!< The in-flight tool execution was aborted
};

/**
 * @brief The ToolResult struct
 *
 * The result of a tool execution. The output is the text that is handed
 * back to the AI model as the tool result, the error carries a failure
 * message when the status is not Success.
 */
struct ToolResult {
    ToolStatus status{ToolStatus::Success};
    std::string output;
    std::string error;

    /**
     * @brief isSuccessful true if the tool run ended with the Success status
     * @return true when the status is ToolStatus::Success
     */
    bool isSuccessful() const {
        return status == ToolStatus::Success;
    }
};

}

#endif // INTERFACES_TOOLRESULT_H