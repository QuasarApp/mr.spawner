//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef NETWORKPROTOCOLTEST_H
#define NETWORKPROTOCOLTEST_H

#include <QtTest>

/**
 * @brief The NetworkProtocolTest class
 *
 * Tests of the network protocol interfaces of the Interfaces module. The
 * tests implement the interfaces with small in-test dummy classes and
 * check that the model query protocol and the protocol manager work as
 * designed.
 */
class NetworkProtocolTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void dummyProtocolContract();
    void protocolConfiguration();
    void protocolModels();
    void protocolConnection();
    void protocolSendDefaults();
    void protocolManagerRegistry();
    void protocolManagerQueries();
    void protocolSignals();
};

#endif // NETWORKPROTOCOLTEST_H