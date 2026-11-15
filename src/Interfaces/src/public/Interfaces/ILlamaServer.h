//
// Copyright (C) 2020-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//


#ifndef INTERFACES_ILLAMASERVER_H
#define INTERFACES_ILLAMASERVER_H

namespace MrSpawner {

/**
 * @brief The ILlamaServer interface
 *
 * The interface of the llama server of the agent. The quick check that runs
 * before the start of the main work cycle (see the QuickCheck of the Core
 * module) shuts the llama server down when there is no work to do, so the
 * resources can be used by other processes.
 */
class ILlamaServer {
public:
    virtual ~ILlamaServer() = default;

    /**
     * @brief start starts the llama server
     * @return true if the llama server was started successfully
     */
    virtual bool start() = 0;

    /**
     * @brief stop stops (shuts down) the llama server
     * @return true if the llama server was stopped successfully
     */
    virtual bool stop() = 0;

    /**
     * @brief isRunning checks if the llama server is running
     * @return true if the llama server is running
     */
    virtual bool isRunning() const = 0;
};

}

#endif // INTERFACES_ILLAMASERVER_H
