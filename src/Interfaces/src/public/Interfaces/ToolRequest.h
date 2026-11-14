//
// Copyright (C) 2020-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef INTERFACES_TOOLREQUEST_H
#define INTERFACES_TOOLREQUEST_H

#include <map>
#include <string>

namespace MrSpawner {

/**
 * @brief The ToolRequest struct
 *
 * A single request to execute a tool of the AI agent. The request is the
 * tool-call object that the AI model produces: the name of the tool to
 * run and its input parameters.
 *
 * The parameters are plain string key-value pairs, the same format the
 * AI model tool implementations use (see the Cline and the
 * llama.qtcreator plugin tools). Complex values (arrays, nested objects)
 * are passed as JSON strings inside the parameter value.
 */
struct ToolRequest {
    std::string tool;
    std::map<std::string, std::string> parameters;
};

}

#endif // INTERFACES_TOOLREQUEST_H