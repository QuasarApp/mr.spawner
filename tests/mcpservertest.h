//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef MCPSERVERTEST_H
#define MCPSERVERTEST_H

#include <QtTest>

class McpServerTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void mcpServerDefaults();
    void mcpServerManagerProtocol();
    void mcpServerSignals();
};

#endif // MCPSERVERTEST_H