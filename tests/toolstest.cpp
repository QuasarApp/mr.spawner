//
// Copyright (C) 2020-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#include "toolstest.h"

#include "Interfaces/ITool.h"
#include "Interfaces/IToolManager.h"
#include "Interfaces/ToolRequest.h"
#include "Interfaces/ToolResult.h"

#include <QHash>
#include <QJsonObject>
#include <QSignalSpy>
#include <QSharedPointer>
#include <QString>
#include <QStringList>

namespace {

/**
 * @brief The DummyTool class
 *
 * A minimal ITool implementation to verify the tool protocol contract.
 */
class DummyTool : public MrSpawner::ITool {
public:
    explicit DummyTool(QObject *parent = nullptr) : MrSpawner::ITool(parent) {
    }

    QString name() const override {
        return QStringLiteral("dummy");
    }

    QString description() const override {
        return QStringLiteral("A dummy tool that echoes the input parameter");
    }

    QString parametersDescription() const override {
        return QStringLiteral(R"({"type":"object","properties":{"input":{"type":"string"}}})");
    }

    MrSpawner::ToolCategory category() const override {
        return MrSpawner::ToolCategory::Execute;
    }

    MrSpawner::ToolResult execute(const MrSpawner::ToolRequest &request) override {
        emit progress(QStringLiteral("working"));
        MrSpawner::ToolResult result;
        if (!request.parameters.contains(QStringLiteral("input"))) {
            result.status = MrSpawner::ToolStatus::Error;
            result.error = QStringLiteral("the input parameter is missing");
            return result;
        }
        result.output = QStringLiteral("dummy:") + request.parameters.value(QStringLiteral("input")).toString();
        return result;
    }
};

/**
 * @brief The DummyToolManager class
 *
 * A minimal in-memory IToolManager implementation to verify the manager
 * protocol contract.
 */
class DummyToolManager : public MrSpawner::IToolManager {
public:
    explicit DummyToolManager(QObject *parent = nullptr) : MrSpawner::IToolManager(parent) {
    }

    bool registerTool(const QSharedPointer<MrSpawner::ITool> &tool) override {
        if (m_tools.contains(tool->name())) {
            return false;
        }
        m_tools.insert(tool->name(), tool);
        emit toolRegistered(tool->name());
        return true;
    }

    bool unregisterTool(const QString &toolName) override {
        if (m_tools.remove(toolName) == 0) {
            return false;
        }
        emit toolUnregistered(toolName);
        return true;
    }

    bool hasTool(const QString &toolName) const override {
        return m_tools.contains(toolName);
    }

    QSharedPointer<MrSpawner::ITool> tool(const QString &toolName) const override {
        return m_tools.value(toolName);
    }

    QStringList tools() const override {
        return m_tools.keys();
    }

    MrSpawner::ToolResult execute(const MrSpawner::ToolRequest &request) override {
        auto it = m_tools.constFind(request.tool);
        if (it == m_tools.constEnd()) {
            MrSpawner::ToolResult result;
            result.status = MrSpawner::ToolStatus::Error;
            result.error = QStringLiteral("unknown tool: ") + request.tool;
            return result;
        }
        return it.value()->execute(request);
    }

private:
    QHash<QString, QSharedPointer<MrSpawner::ITool>> m_tools;
};

}

void ToolsTest::initTestCase() {
}

void ToolsTest::cleanupTestCase() {
}

void ToolsTest::toolRequestDefaults() {
    MrSpawner::ToolRequest request;
    QVERIFY(request.tool.isEmpty());
    QVERIFY(request.parameters.isEmpty());

    request.tool = QStringLiteral("dummy");
    QJsonObject parameters;
    parameters.insert(QStringLiteral("input"), QStringLiteral("test"));
    request.parameters = parameters;
    QCOMPARE(request.tool, QString("dummy"));
    QCOMPARE(request.parameters.value(QStringLiteral("input")).toString(), QString("test"));
}

void ToolsTest::toolResultDefaults() {
    MrSpawner::ToolResult result;
    QCOMPARE(static_cast<int>(result.status), static_cast<int>(MrSpawner::ToolStatus::Success));
    QVERIFY(result.isSuccessful());
    QVERIFY(result.output.isEmpty());
    QVERIFY(result.error.isEmpty());

    MrSpawner::ToolResult errorResult;
    errorResult.status = MrSpawner::ToolStatus::Error;
    errorResult.error = QStringLiteral("something failed");
    QVERIFY(!errorResult.isSuccessful());

    MrSpawner::ToolResult rejectedResult;
    rejectedResult.status = MrSpawner::ToolStatus::Rejected;
    QVERIFY(!rejectedResult.isSuccessful());

    MrSpawner::ToolResult abortedResult;
    abortedResult.status = MrSpawner::ToolStatus::Aborted;
    QVERIFY(!abortedResult.isSuccessful());
}

void ToolsTest::dummyToolContract() {
    auto tool = QSharedPointer<MrSpawner::ITool>(new DummyTool);

    QCOMPARE(tool->name(), QString("dummy"));
    QCOMPARE(tool->displayName(), tool->name());
    QVERIFY(!tool->description().isEmpty());
    QVERIFY(!tool->parametersDescription().isEmpty());
    QVERIFY(tool->category() == MrSpawner::ToolCategory::Execute);
    QVERIFY(!tool->requiresApproval());

    MrSpawner::ToolRequest request;
    request.tool = tool->name();
    QJsonObject parameters;
    parameters.insert(QStringLiteral("input"), QStringLiteral("value"));
    request.parameters = parameters;
    MrSpawner::ToolResult result = tool->execute(request);
    QVERIFY(result.isSuccessful());
    QCOMPARE(result.output, QString("dummy:value"));

    // A request without the required parameter must report an error.
    MrSpawner::ToolRequest emptyRequest;
    emptyRequest.tool = tool->name();
    MrSpawner::ToolResult errorResult = tool->execute(emptyRequest);
    QVERIFY(!errorResult.isSuccessful());
    QCOMPARE(static_cast<int>(errorResult.status), static_cast<int>(MrSpawner::ToolStatus::Error));
    QVERIFY(!errorResult.error.isEmpty());
}

void ToolsTest::toolManagerProtocol() {
    DummyToolManager manager;
    QVERIFY(manager.tools().isEmpty());
    QVERIFY(!manager.hasTool("dummy"));
    QVERIFY(!manager.tool("dummy"));

    auto tool = QSharedPointer<MrSpawner::ITool>(new DummyTool);
    QVERIFY(manager.registerTool(tool));
    QVERIFY(manager.hasTool("dummy"));
    QCOMPARE(manager.tools().size(), 1);
    QCOMPARE(manager.tool("dummy")->name(), QString("dummy"));

    // A duplicate registration must be rejected.
    auto duplicate = QSharedPointer<MrSpawner::ITool>(new DummyTool);
    QVERIFY(!manager.registerTool(duplicate));

    MrSpawner::ToolRequest request;
    request.tool = "dummy";
    QJsonObject parameters;
    parameters.insert(QStringLiteral("input"), QStringLiteral("manager"));
    request.parameters = parameters;
    MrSpawner::ToolResult result = manager.execute(request);
    QVERIFY(result.isSuccessful());
    QCOMPARE(result.output, QString("dummy:manager"));

    // An unknown tool must produce an error result.
    MrSpawner::ToolRequest unknown;
    unknown.tool = "unknown";
    MrSpawner::ToolResult errorResult = manager.execute(unknown);
    QVERIFY(!errorResult.isSuccessful());
    QCOMPARE(static_cast<int>(errorResult.status), static_cast<int>(MrSpawner::ToolStatus::Error));
    QVERIFY(!errorResult.error.isEmpty());

    QVERIFY(manager.unregisterTool("dummy"));
    QVERIFY(!manager.unregisterTool("dummy"));
    QVERIFY(!manager.hasTool("dummy"));
}

void ToolsTest::toolSignals() {
    // The tool must report its progress through the QObject signal.
    auto tool = QSharedPointer<MrSpawner::ITool>(new DummyTool);
    QSignalSpy progressSpy(tool.data(), &MrSpawner::ITool::progress);
    QVERIFY(progressSpy.isValid());

    MrSpawner::ToolRequest request;
    request.tool = tool->name();
    QJsonObject parameters;
    parameters.insert(QStringLiteral("input"), QStringLiteral("spy"));
    request.parameters = parameters;
    tool->execute(request);
    QCOMPARE(progressSpy.count(), 1);
    QCOMPARE(progressSpy.at(0).at(0).toString(), QString("working"));

    // The manager must announce registry changes through the QObject signals.
    DummyToolManager manager;
    QSignalSpy registeredSpy(&manager, &MrSpawner::IToolManager::toolRegistered);
    QSignalSpy unregisteredSpy(&manager, &MrSpawner::IToolManager::toolUnregistered);
    QVERIFY(registeredSpy.isValid());
    QVERIFY(unregisteredSpy.isValid());

    QVERIFY(manager.registerTool(QSharedPointer<MrSpawner::ITool>(new DummyTool)));
    QCOMPARE(registeredSpy.count(), 1);
    QCOMPARE(registeredSpy.at(0).at(0).toString(), QString("dummy"));

    QVERIFY(manager.unregisterTool("dummy"));
    QCOMPARE(unregisteredSpy.count(), 1);
    QCOMPARE(unregisteredSpy.at(0).at(0).toString(), QString("dummy"));
}

// Standard stand-alone QTest entry point, see the Qt test tutorial:
// https://doc.qt.io/qt-6/qttestlib-tutorial1-example.html
QTEST_GUILESS_MAIN(ToolsTest)