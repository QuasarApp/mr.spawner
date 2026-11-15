//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef INTERFACES_TOOLREQUEST_H
#define INTERFACES_TOOLREQUEST_H

#include <QJsonObject>
#include <QString>

namespace MrSpawner {

/**
 * @brief The ToolRequest struct
 *
 * A single request to execute a tool of the AI agent. The request is the
 * tool-call object that the AI model produces: the name of the tool to
 * run and its input parameters.
 *
 * The parameters are a JSON object (QJsonObject), the same format the AI
 * model tool implementations use (see the Cline and the llama.qtcreator
 * plugin tools). A parameter value may be any JSON value type: string,
 * number, boolean, array or a nested object.
 */
struct ToolRequest {
    QString tool;             //!< The name of the tool to execute
    QJsonObject parameters;   //!< The input parameters of the tool call
};

}

#endif // INTERFACES_TOOLREQUEST_H