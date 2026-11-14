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

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace {

/**
 * @brief The DummyTool class
 *
 * A minimal ITool implementation to verify the tool protocol contract.
 */
class DummyTool : public MrSpawner::ITool {
public:
    std::string name() const override {
        return "dummy";
    }

    std::string description() const override {
        return "A dummy tool that echoes the input parameter";
    }

    std::string parametersDescription() const override {
        return R"({"type":"object","properties":{"input":{"type":"string"}}})";
    }

    MrSpawner::ToolCategory category() const override {
        return MrSpawner::ToolCategory::Execute;
    }

    MrSpawner::ToolResult execute(const MrSpawner::ToolRequest &request) override {
        MrSpawner::ToolResult result;
        auto it = request.parameters.find("input");
        if (it == request.parameters.end()) {
            result.status = MrSpawner::ToolStatus::Error;
            result.error = "the input parameter is missing";
            return result;
        }
        result.output = "dummy:" + it->second;
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
    bool registerTool(const std::shared_ptr<MrSpawner::ITool> &tool) override {
        if (m_tools.count(tool->name()) > 0) {
            return false;
        }
        m_tools[tool->name()] = tool;
        return true;
    }

    bool unregisterTool(const std::string &toolName) override {
        return m_tools.erase(toolName) > 0;
    }

    bool hasTool(const std::string &toolName) const override {
        return m_tools.count(toolName) > 0;
    }

    std::shared_ptr<MrSpawner::ITool> tool(const std::string &toolName) const override {
        auto it = m_tools.find(toolName);
        return it == m_tools.end() ? nullptr : it->second;
    }

    std::vector<std::string> tools() const override {
        std::vector<std::string> names;
        names.reserve(m_tools.size());
        for (const auto &pair : m_tools) {
            names.push_back(pair.first);
        }
        return names;
    }

    MrSpawner::ToolResult execute(const MrSpawner::ToolRequest &request) override {
        auto it = m_tools.find(request.tool);
        if (it == m_tools.end()) {
            MrSpawner::ToolResult result;
            result.status = MrSpawner::ToolStatus::Error;
            result.error = "unknown tool: " + request.tool;
            return result;
        }
        return it->second->execute(request);
    }

private:
    std::map<std::string, std::shared_ptr<MrSpawner::ITool>> m_tools;
};

}

void ToolsTest::initTestCase() {
}

void ToolsTest::cleanupTestCase() {
}

void ToolsTest::toolRequestDefaults() {
    MrSpawner::ToolRequest request;
    QVERIFY(request.tool.empty());
    QVERIFY(request.parameters.empty());

    request.tool = "dummy";
    request.parameters["input"] = "test";
    QCOMPARE(request.tool, std::string("dummy"));
    QCOMPARE(request.parameters.at("input"), std::string("test"));
}

void ToolsTest::toolResultDefaults() {
    MrSpawner::ToolResult result;
    QCOMPARE(static_cast<int>(result.status), static_cast<int>(MrSpawner::ToolStatus::Success));
    QVERIFY(result.isSuccessful());
    QVERIFY(result.output.empty());
    QVERIFY(result.error.empty());

    MrSpawner::ToolResult errorResult;
    errorResult.status = MrSpawner::ToolStatus::Error;
    errorResult.error = "something failed";
    QVERIFY(!errorResult.isSuccessful());

    MrSpawner::ToolResult rejectedResult;
    rejectedResult.status = MrSpawner::ToolStatus::Rejected;
    QVERIFY(!rejectedResult.isSuccessful());

    MrSpawner::ToolResult abortedResult;
    abortedResult.status = MrSpawner::ToolStatus::Aborted;
    QVERIFY(!abortedResult.isSuccessful());
}

void ToolsTest::dummyToolContract() {
    auto tool = std::make_shared<DummyTool>();

    QCOMPARE(tool->name(), std::string("dummy"));
    QCOMPARE(tool->displayName(), tool->name());
    QVERIFY(!tool->description().empty());
    QVERIFY(!tool->parametersDescription().empty());
    QVERIFY(tool->category() == MrSpawner::ToolCategory::Execute);
    QVERIFY(!tool->requiresApproval());

    MrSpawner::ToolRequest request;
    request.tool = tool->name();
    request.parameters["input"] = "value";
    MrSpawner::ToolResult result = tool->execute(request);
    QVERIFY(result.isSuccessful());
    QCOMPARE(result.output, std::string("dummy:value"));

    // A request without the required parameter must report an error.
    MrSpawner::ToolRequest emptyRequest;
    emptyRequest.tool = tool->name();
    MrSpawner::ToolResult errorResult = tool->execute(emptyRequest);
    QVERIFY(!errorResult.isSuccessful());
    QCOMPARE(static_cast<int>(errorResult.status), static_cast<int>(MrSpawner::ToolStatus::Error));
    QVERIFY(!errorResult.error.empty());
}

void ToolsTest::toolManagerProtocol() {
    DummyToolManager manager;
    QVERIFY(manager.tools().empty());
    QVERIFY(!manager.hasTool("dummy"));
    QVERIFY(manager.tool("dummy") == nullptr);

    auto tool = std::make_shared<DummyTool>();
    QVERIFY(manager.registerTool(tool));
    QVERIFY(manager.hasTool("dummy"));
    QCOMPARE(manager.tools().size(), 1u);
    QCOMPARE(manager.tool("dummy")->name(), std::string("dummy"));

    // A duplicate registration must be rejected.
    auto duplicate = std::make_shared<DummyTool>();
    QVERIFY(!manager.registerTool(duplicate));

    MrSpawner::ToolRequest request;
    request.tool = "dummy";
    request.parameters["input"] = "manager";
    MrSpawner::ToolResult result = manager.execute(request);
    QVERIFY(result.isSuccessful());
    QCOMPARE(result.output, std::string("dummy:manager"));

    // An unknown tool must produce an error result.
    MrSpawner::ToolRequest unknown;
    unknown.tool = "unknown";
    MrSpawner::ToolResult errorResult = manager.execute(unknown);
    QVERIFY(!errorResult.isSuccessful());
    QCOMPARE(static_cast<int>(errorResult.status), static_cast<int>(MrSpawner::ToolStatus::Error));
    QVERIFY(!errorResult.error.empty());

    QVERIFY(manager.unregisterTool("dummy"));
    QVERIFY(!manager.unregisterTool("dummy"));
    QVERIFY(!manager.hasTool("dummy"));
}

// Standard stand-alone QTest entry point, see the Qt test tutorial:
// https://doc.qt.io/qt-6/qttestlib-tutorial1-example.html
QTEST_GUILESS_MAIN(ToolsTest)