//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#include "mcpservertest.h"

#include "Interfaces/McpServer.h"
#include "Interfaces/IMcpServerManager.h"

#include <QHash>
#include <QJsonObject>
#include <QSignalSpy>
#include <QSharedPointer>
#include <QString>
#include <QStringList>

namespace {

class DummyMCPServer : public MrSpawner::MCPServer {
public:
    explicit DummyMCPServer(QObject *parent = nullptr) : MrSpawner::MCPServer(parent) {
    }

    QString name() const override {
        return QStringLiteral("dummy_mcp_server");
    }

    QString description() const override {
        return QStringLiteral("A dummy MCP server for testing");
    }

    QString capabilitiesDescription() const override {
        return QStringLiteral(R"({ "type": "object", "properties": { "tools": { "type": "array" } } })");
    }

    MrSpawner::MCPPurpose purpose() const override {
        return MrSpawner::MCPPurpose::Data;
    }

    bool requiresApproval() const override {
        return false;
    }

    MrSpawner::ToolResult connect(const MrSpawner::ToolRequest &) override {
        emit progress(QStringLiteral("connecting to dummy MCP server"));
        emit capabilitiesReady(QStringList() << "tools" << "prompts");
        return MrSpawner::ToolResult();
    }

    MrSpawner::QStringList availableCapabilities() const override {
        return QStringList() << "tools" << "prompts";
    }
};

class DummyMCPServerManager : public MrSpawner::IMcpServerManager {
public:
    explicit DummyMCPServerManager(QObject *parent = nullptr) : MrSpawner::IMcpServerManager(parent) {
    }

    bool registerServer(const QSharedPointer<MrSpawner::MCPServer> &server) override {
        if (m_servers.contains(server->name())) {
            return false;
        }
        m_servers.insert(server->name(), server);
        emit serverRegistered(server->name());
        return true;
    }

    bool unregisterServer(const QString &serverName) override {
        if (m_servers.remove(serverName) == 0) {
            return false;
        }
        emit serverDisconnected(serverName);
        return true;
    }

    bool hasServer(const QString &serverName) const override {
        return m_servers.contains(serverName);
    }

    QSharedPointer<MrSpawner::MCPServer> server(const QString &serverName) const override {
        return m_servers.value(serverName);
    }

    QStringList servers() const override {
        return m_servers.keys();
    }

    MrSpawner::ToolResult connect(const MrSpawner::ToolRequest &) override {
        return MrSpawner::ToolResult();
    }

    MrSpawner::ToolResult disconnect(const QString &serverName) override {
        return MrSpawner::ToolResult();
    }

private:
    QHash<QString, QSharedPointer<MrSpawner::MCPServer>> m_servers;
};

} // namespace

void McpServerTest::initTestCase() {
}

void McpServerTest::cleanupTestCase() {
}

void McpServerTest::mcpServerDefaults() {
    DummyMCPServer server;

    QCOMPARE(server.name(), QString("dummy_mcp_server"));
    QVERIFY(!server.name().isEmpty());
    QCOMPARE(server.description(), QString("A dummy MCP server for testing"));
    QVERIFY(!server.description().isEmpty());
    QCOMPARE(server.capabilitiesDescription(), QString(R"({ "type": "object", "properties": { "tools": { "type": "array" } } })"));
    QVERIFY(!server.capabilitiesDescription().isEmpty());
    QCOMPARE(server.displayName(), QString("dummy_mcp_server"));
    QCOMPARE(server.purpose(), MrSpawner::MCPPurpose::Data);
    QCOMPARE(server.requiresApproval(), false);
    QCOMPARE(server.isConnected(), false);
    MrSpawner::ToolResult connection = server.connect(MrSpawner::ToolRequest());
    QCOMPARE(connection.status, MrSpawner::ToolStatus::Success);
}

void McpServerTest::mcpServerManagerProtocol() {
    DummyMCPServerManager manager;
    QVERIFY(manager.servers().isEmpty());
    QVERIFY(!manager.hasServer("dummy_mcp_server"));
    QVERIFY(!manager.server("dummy_mcp_server"));

    auto server = QSharedPointer<MrSpawner::MCPServer>(new DummyMCPServer);
    QVERIFY(manager.registerServer(server));
    QVERIFY(manager.hasServer("dummy_mcp_server"));
    QCOMPARE(manager.servers().size(), 1);
    QCOMPARE(manager.server("dummy_mcp_server")->name(), QString("dummy_mcp_server"));

    auto duplicate = QSharedPointer<MrSpawner::MCPServer>(new DummyMCPServer);
    QVERIFY(!manager.registerServer(duplicate));

    MrSpawner::ToolRequest request;
    request.tool = "dummy_tool";
    QJsonObject parameters;
    parameters.insert(QStringLiteral("query"), QStringLiteral("test"));
    request.parameters = parameters;
    MrSpawner::ToolResult result = manager.connect(request);
    QVERIFY(result.isSuccessful());

    QVERIFY(manager.unregisterServer("dummy_mcp_server"));
    QVERIFY(!manager.unregisterServer("dummy_mcp_server"));
    QVERIFY(!manager.hasServer("dummy_mcp_server"));
}

void McpServerTest::mcpServerSignals() {
    auto server = QSharedPointer<MrSpawner::MCPServer>(new DummyMCPServer);
    QSignalSpy progressSpy(server.data(), &MrSpawner::MCPServer::progress);
    QVERIFY(progressSpy.isValid());

    DummyMCPServerManager manager;
    QSignalSpy registeredSpy(&manager, &MrSpawner::IMcpServerManager::serverRegistered);
    QSignalSpy disconnectedSpy(&manager, &MrSpawner::IMcpServerManager::serverDisconnected);
    QVERIFY(registeredSpy.isValid());
    QVERIFY(disconnectedSpy.isValid());

    QVERIFY(manager.registerServer(QSharedPointer<MrSpawner::MCPServer>(new DummyMCPServer)));
    QCOMPARE(registeredSpy.count(), 1);
    QCOMPARE(registeredSpy.at(0).at(0).toString(), QString("dummy_mcp_server"));

    QVERIFY(manager.unregisterServer("dummy_mcp_server"));
    QCOMPARE(disconnectedSpy.count(), 1);
    QCOMPARE(disconnectedSpy.at(0).at(0).toString(), QString("dummy_mcp_server"));
}

// Standard stand-alone QTest entry point
QTEST_GUILESS_MAIN(McpServerTest)
