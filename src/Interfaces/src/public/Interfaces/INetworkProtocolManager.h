//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef INTERFACES_INETWORKPROTOCOLMANAGER_H
#define INTERFACES_INETWORKPROTOCOLMANAGER_H

#include "Interfaces/INetworkProtocol.h"

#include <QSharedPointer>
#include <QString>
#include <QStringList>

namespace MrSpawner {

/**
 * @brief The INetworkProtocolManager interface
 *
 * The central registry and executor of the network protocols. Protocols
 * register themselves here and the agent sends model queries through this
 * interface. The Core module provides the implementation of this
 * interface.
 *
 * INetworkProtocolManager is a QObject, the Qt base class of the project:
 * registry changes are announced to the agent through the protocolRegistered
 * and protocolUnregistered signals.
 */
class INetworkProtocolManager : public QObject {
    Q_OBJECT
public:
    virtual ~INetworkProtocolManager() = default;

    /**
     * @brief registerProtocol adds a network protocol to the registry
     * @param protocol the protocol to register
     * @return false when a protocol with the same name is already registered
     */
    virtual bool registerProtocol(const QSharedPointer<INetworkProtocol> &protocol) = 0;

    /**
     * @brief unregisterProtocol removes a network protocol from the registry
     * @param protocolName the name of the protocol to remove
     * @return false when no protocol with that name is registered
     */
    virtual bool unregisterProtocol(const QString &protocolName) = 0;

    /**
     * @brief hasProtocol true if a network protocol with the given name is registered
     * @param protocolName the name of the protocol
     * @return true when the protocol is registered
     */
    virtual bool hasProtocol(const QString &protocolName) const = 0;

    /**
     * @brief protocol returns the registered network protocol
     * @param protocolName the name of the protocol
     * @return the protocol or a null QSharedPointer when the name is unknown
     */
    virtual QSharedPointer<INetworkProtocol> protocol(const QString &protocolName) const = 0;

    /**
     * @brief protocols the names of all registered network protocols
     * @return the list of protocol names
     */
    virtual QStringList protocols() const = 0;

    /**
     * @brief defaultProtocol the protocol that is used for new model queries
     * @return the default protocol or a null QSharedPointer when no default is set
     */
    virtual QSharedPointer<INetworkProtocol> defaultProtocol() const = 0;

    /**
     * @brief setDefaultProtocol sets the protocol that is used for new model queries
     * @param protocolName the name of the protocol to set as default
     * @return false when no protocol with that name is registered
     */
    virtual bool setDefaultProtocol(const QString &protocolName) = 0;

    /**
     * @brief query executes a synchronous model query through the default protocol
     * @param request the model query request
     * @return the model query result; an Error result when no default protocol is set
     */
    virtual ToolResult query(const ToolRequest &request) = 0;

    /**
     * @brief send executes an asynchronous model query through the default protocol
     * @param request the model query request
     * @return the send result; an Error result when no default protocol is set
     */
    virtual ToolResult send(const ToolRequest &request) = 0;

signals:
    /**
     * @brief protocolRegistered a new network protocol was added to the registry
     * @param protocolName the name of the registered protocol
     */
    void protocolRegistered(const QString &protocolName);

    /**
     * @brief protocolUnregistered a network protocol was removed from the registry
     * @param protocolName the name of the removed protocol
     */
    void protocolUnregistered(const QString &protocolName);

    /**
     * @brief defaultProtocolChanged the default network protocol was changed
     * @param protocolName the name of the new default protocol
     */
    void defaultProtocolChanged(const QString &protocolName);

protected:
    explicit INetworkProtocolManager(QObject *parent = nullptr) : QObject(parent) {
    }
};

}

#endif // INTERFACES_INETWORKPROTOCOLMANAGER_H
