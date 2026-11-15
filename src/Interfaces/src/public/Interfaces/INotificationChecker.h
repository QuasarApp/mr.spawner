//
// Copyright (C) 2020-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//


#ifndef INTERFACES_INOTIFICATIONCHECKER_H
#define INTERFACES_INOTIFICATIONCHECKER_H

namespace MrSpawner {

/**
 * @brief The INotificationChecker interface
 *
 * The provider of the agent notifications. The quick check that runs before
 * the start of the main work cycle (see the QuickCheck of the Core module)
 * uses this interface to detect the unread notifications and to mark the
 * notifications as read.
 */
class INotificationChecker {
public:
    virtual ~INotificationChecker() = default;

    /**
     * @brief hasUnreadNotifications checks if there are unread notifications
     * @return true if at least one notification is still unread
     */
    virtual bool hasUnreadNotifications() const = 0;

    /**
     * @brief markAllAsRead marks ALL notifications as read
     *
     * All notifications, not only the unread ones, are marked as read, so
     * the next cycle only sees the new notifications.
     * @return true if all notifications were marked as read
     */
    virtual bool markAllAsRead() = 0;
};

}

#endif // INTERFACES_INOTIFICATIONCHECKER_H
