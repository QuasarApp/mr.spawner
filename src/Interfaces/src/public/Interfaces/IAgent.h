//
// Copyright (C) 2020-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef INTERFACES_IAGENT_H
#define INTERFACES_IAGENT_H

namespace MrSpawner {

/**
 * @brief The IAgent interface is the main public interface of the AI agent.
 *
 * The Core module provides the implementation of this interface.
 */
class IAgent {
public:
    virtual ~IAgent() = default;
};

}

#endif // INTERFACES_IAGENT_H
