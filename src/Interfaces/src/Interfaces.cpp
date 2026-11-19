//
// Copyright (C) 2020-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

// The Interfaces module contains only the public interfaces of the project.
// This translation unit includes the QObject based interface headers so
// that their moc files are compiled into the library, and keeps the
// library target buildable.
#include "Interfaces/IAgent.h"
#include "Interfaces/IPlugin.h"
#include "Interfaces/IPluginManager.h"
#include "Interfaces/ITool.h"
#include "Interfaces/IToolManager.h"
#include "Interfaces/INetworkProtocol.h"
#include "Interfaces/INetworkProtocolManager.h"
#include "Interfaces/ToolRequest.h"
#include "Interfaces/ToolResult.h"
