//
// Copyright (C) 2020-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//


#ifndef CORE_QUICKCHECK_H
#define CORE_QUICKCHECK_H

#include "Interfaces/ILlamaServer.h"
#include "Interfaces/INotificationChecker.h"

#include <chrono>

namespace MrSpawner {

/**
 * @brief The QuickCheckResult enum
 *
 * The result of the quick check that runs before the start of the main
 * work cycle of the agent.
 */
enum class QuickCheckResult {
    ContinueWork,  //!< There are unread notifications: continue the work (the current agent logic)
    Sleep          //!< There are no unread notifications: sleep for the configured timeout
};

/**
 * @brief The QuickCheck class
 *
 * The quick check that runs before the start of the main work cycle of the
 * agent (issue #7):
 *
 * - If there are unread notifications the agent continues the work with the
 *   current logic. ALL notifications are marked as read, so the next cycle
 *   only sees the new notifications.
 * - If there are no unread notifications the llama server is shut down and
 *   the agent goes to sleep for the configured timeout.
 */
class QuickCheck {
public:
    /**
     * @brief QuickCheck constructor
     * @param checker the provider of the agent notifications
     * @param llama the llama server that is shut down when the agent goes to sleep
     * @param sleepTimeout the timeout of the sleep when there is no work to do
     */
    QuickCheck(INotificationChecker* checker,
               ILlamaServer* llama,
               std::chrono::seconds sleepTimeout = std::chrono::seconds(3600));

    /**
     * @brief run the quick check before the start of the main work cycle
     *
     * If there are unread notifications, all notifications are marked as
     * read and QuickCheckResult::ContinueWork is returned. The caller
     * continues the work with the current agent logic.
     *
     * If there are no unread notifications, the llama server is shut down,
     * the agent sleeps for the configured timeout and
     * QuickCheckResult::Sleep is returned.
     * @return the result of the quick check
     */
    QuickCheckResult run();

    /**
     * @brief sleepTimeout the configured sleep timeout
     * @return the sleep timeout
     */
    std::chrono::seconds sleepTimeout() const { return m_sleepTimeout; }

private:
    INotificationChecker* m_checker = nullptr;
    ILlamaServer* m_llama = nullptr;
    std::chrono::seconds m_sleepTimeout = std::chrono::seconds(3600);
};

}

#endif // CORE_QUICKCHECK_H
