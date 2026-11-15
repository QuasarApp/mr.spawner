//
// Copyright (C) 2020-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//


#include "Core/QuickCheck.h"

#include <thread>

namespace MrSpawner {

QuickCheck::QuickCheck(INotificationChecker* checker,
                       ILlamaServer* llama,
                       std::chrono::seconds sleepTimeout)
    : m_checker(checker)
    , m_llama(llama)
    , m_sleepTimeout(sleepTimeout)
{
}

QuickCheckResult QuickCheck::run() {
    // There are unread notifications: continue the work with the current
    // agent logic and mark ALL notifications as read, so the next cycle
    // only sees the new notifications.
    if (m_checker != nullptr && m_checker->hasUnreadNotifications()) {
        m_checker->markAllAsRead();
        return QuickCheckResult::ContinueWork;
    }

    // There are no unread notifications: shut down the llama server to save
    // resources and go to sleep for the configured timeout.
    if (m_llama != nullptr) {
        m_llama->stop();
    }
    std::this_thread::sleep_for(m_sleepTimeout);
    return QuickCheckResult::Sleep;
}

}
