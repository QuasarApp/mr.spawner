//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#include "networkprotocoltest.h"

#include "Interfaces/INetworkProtocol.h"
#include "Interfaces/INetworkProtocolManager.h"

#include <QHash>
#include <QJsonObject>
#include <QSignalSpy>
#include <QSharedPointer>
#include <QString>
#include <QStringList>

namespace {

/**
 * @brief The DummyProtocol class
 *
 * A full INetworkProtocol implementation to verify the protocol contract.
 */
class DummyProtocol : public MrSpawner::INetworkProtocol {
public:
    DummyProtocol(const QString &name, MrSpawner::ProtocolType type, QObject *parent = nullptr)
        : MrSpawner::INetworkProtocol(parent), m_name(name), m_type(type) {
    }

    MrSpawner::ProtocolType type() const override {
        return m_type;
    }

    QString name() const override {
        return m_name;
    }

    QString description() const override {
        return "A dummy " + m_name + " protocol";
    }

    QJsonObject configuration() const override {
        return m_configuration;
    }

    bool configure(const QJsonObject &configuration) override {
        if (!configuration.contains("endpoint")) {
            return false;
        }
        m_configuration = configuration;
        return true;
    }

    QStringList models() const override {
        return m_models;
    }

    QString activeModel() const override {
        return m_activeModel;
    }

    bool selectModel(const QString &model) override {
        if (!m_models.contains(model)) {
            return false;
        }
        m_activeModel = model;
        return true;
    }

    MrSpawner::ToolResult connect() override {
        emit progress("connecting " + m_name);
        m_connected = true;
        return MrSpawner::ToolResult();
    }

    MrSpawner::ToolResult disconnect() override {
        m_connected = false;
        return MrSpawner::ToolResult();
    }

    bool isConnected() const override {
        return m_connected;
    }

    MrSpawner::ToolResult query(const MrSpawner::ToolRequest &request) override {
        emit progress("querying " + m_name);
        MrSpawner::ToolResult result;
        if (!request.parameters.contains("prompt")) {
            result.status = MrSpawner::ToolStatus::Error;
            result.error = "the prompt parameter is missing";
            return result;
        }
        emit response(m_name + ":");
        emit response(request.parameters.value("prompt").toString());
        result.output = m_name + ":" + request.parameters.value("prompt").toString();
        return result;
    }

    void raiseConnectionError() {
        emit connectionError("dummy connection error");
    }

private:
    QString m_name;
    MrSpawner::ProtocolType m_type;
    QJsonObject m_configuration;
    QStringList m_models{"model-a", "model-b"};
    QString m_activeModel{"model-a"};
    bool m_connected{false};
};

/**
 * @brief The MinimalProtocol class
 *
 * A minimal INetworkProtocol implementation that only overrides the pure
 * virtual methods, to verify the base class defaults.
 */
class MinimalProtocol : public MrSpawner::INetworkProtocol {
public:
    explicit MinimalProtocol(QObject *parent = nullptr) : MrSpawner::INetworkProtocol(parent) {
    }

    MrSpawner::ProtocolType type() const override {
        return MrSpawner::ProtocolType::LlamaCpp;
    }

    QString name() const override {
        return "minimal";
    }

    QString description() const override {
        return "A minimal protocol";
    }

    MrSpawner::ToolResult query(const MrSpawner::ToolRequest &request) override {
        MrSpawner::ToolResult result;
        result.output = "minimal:" + request.parameters.value("prompt").toString();
        return result;
    }
};

/**
 * @brief The DummyProtocolManager class
 *
 * A minimal in-memory INetworkProtocolManager implementation to verify the
 * manager protocol contract.
 */
class DummyProtocolManager : public MrSpawner::INetworkProtocolManager {
public:
    explicit DummyProtocolManager(QObject *parent = nullptr) : MrSpawner::INetworkProtocolManager(parent) {
    }

    bool registerProtocol(const QSharedPointer<MrSpawner::INetworkProtocol> &protocol) override {
        if (m_protocols.contains(protocol->name())) {
            return false;
        }
        m_protocols.insert(protocol->name(), protocol);
        emit protocolRegistered(protocol->name());
        return true;
    }

    bool unregisterProtocol(const QString &protocolName) override {
        if (m_protocols.remove(protocolName) == 0) {
            return false;
        }
        if (m_defaultProtocol == protocolName) {
            m_defaultProtocol.clear();
            emit defaultProtocolChanged(m_defaultProtocol);
        }
        emit protocolUnregistered(protocolName);
        return true;
    }

    bool hasProtocol(const QString &protocolName) const override {
        return m_protocols.contains(protocolName);
    }

    QSharedPointer<MrSpawner::INetworkProtocol> protocol(const QString &protocolName) const override {
        return m_protocols.value(protocolName);
    }

    QStringList protocols() const override {
        return m_protocols.keys();
    }

    QSharedPointer<MrSpawner::INetworkProtocol> defaultProtocol() const override {
        return m_protocols.value(m_defaultProtocol);
    }

    bool setDefaultProtocol(const QString &protocolName) override {
        if (!m_protocols.contains(protocolName)) {
            return false;
        }
        m_defaultProtocol = protocolName;
        emit defaultProtocolChanged(protocolName);
        return true;
    }

    MrSpawner::ToolResult query(const MrSpawner::ToolRequest &request) override {
        auto protocol = defaultProtocol();
        if (!protocol) {
            MrSpawner::ToolResult result;
            result.status = MrSpawner::ToolStatus::Error;
            result.error = "no default protocol is set";
            return result;
        }
        return protocol->query(request);
    }

    MrSpawner::ToolResult send(const MrSpawner::ToolRequest &request) override {
        auto protocol = defaultProtocol();
        if (!protocol) {
            MrSpawner::ToolResult result;
            result.status = MrSpawner::ToolStatus::Error;
            result.error = "no default protocol is set";
            return result;
        }
        return protocol->send(request);
    }

private:
    QHash<QString, QSharedPointer<MrSpawner::INetworkProtocol>> m_protocols;
    QString m_defaultProtocol;
};

}

void NetworkProtocolTest::initTestCase() {
}

void NetworkProtocolTest::cleanupTestCase() {
}

void NetworkProtocolTest::dummyProtocolContract() {
    // The four provider protocols must be distinguishable from each other
    // and from the unknown type.
    QCOMPARE(static_cast<int>(MrSpawner::ProtocolType::Unknown), 0);
    QVERIFY(MrSpawner::ProtocolType::LlamaCpp != MrSpawner::ProtocolType::Claude);
    QVERIFY(MrSpawner::ProtocolType::Claude != MrSpawner::ProtocolType::Gemini);
    QVERIFY(MrSpawner::ProtocolType::Gemini != MrSpawner::ProtocolType::Gpt);

    auto protocol = QSharedPointer<MrSpawner::INetworkProtocol>(new DummyProtocol("gpt", MrSpawner::ProtocolType::Gpt));
    QCOMPARE(protocol->type(), MrSpawner::ProtocolType::Gpt);
    QCOMPARE(protocol->name(), QString("gpt"));
    QCOMPARE(protocol->displayName(), QString("gpt"));
    QVERIFY(!protocol->description().isEmpty());

    // A query without the required parameter must report an error.
    MrSpawner::ToolRequest emptyRequest;
    emptyRequest.tool = protocol->name();
    MrSpawner::ToolResult errorResult = protocol->query(emptyRequest);
    QVERIFY(!errorResult.isSuccessful());
    QCOMPARE(static_cast<int>(errorResult.status), static_cast<int>(MrSpawner::ToolStatus::Error));
    QVERIFY(!errorResult.error.isEmpty());

    // A query with the prompt must return the model answer.
    MrSpawner::ToolRequest request;
    request.tool = protocol->name();
    QJsonObject parameters;
    parameters.insert("prompt", "hello");
    request.parameters = parameters;
    MrSpawner::ToolResult result = protocol->query(request);
    QVERIFY(result.isSuccessful());
    QCOMPARE(result.output, QString("gpt:hello"));
}

void NetworkProtocolTest::protocolConfiguration() {
    // The base class defaults: no configuration and configure() is rejected.
    MinimalProtocol minimal;
    QVERIFY(minimal.configuration().isEmpty());
    QJsonObject rejected;
    QVERIFY(!minimal.configure(rejected));

    // A configured protocol must return its configuration.
    auto protocol = QSharedPointer<MrSpawner::INetworkProtocol>(new DummyProtocol("claude", MrSpawner::ProtocolType::Claude));
    QJsonObject configuration;
    configuration.insert("endpoint", "https://api.anthropic.com");
    configuration.insert("model", "claude-sonnet");
    QVERIFY(protocol->configure(configuration));
    QCOMPARE(protocol->configuration().value("endpoint").toString(), QString("https://api.anthropic.com"));

    // A configuration without the required endpoint must be rejected.
    QJsonObject incomplete;
    incomplete.insert("model", "claude-sonnet");
    QVERIFY(!protocol->configure(incomplete));
}

void NetworkProtocolTest::protocolModels() {
    // The base class defaults: no model selection.
    MinimalProtocol minimal;
    QVERIFY(minimal.models().isEmpty());
    QVERIFY(minimal.activeModel().isEmpty());
    QVERIFY(!minimal.selectModel("any-model"));

    auto protocol = QSharedPointer<MrSpawner::INetworkProtocol>(new DummyProtocol("gemini", MrSpawner::ProtocolType::Gemini));
    QCOMPARE(protocol->models().size(), 2);
    QVERIFY(protocol->models().contains("model-a"));
    QCOMPARE(protocol->activeModel(), QString("model-a"));

    // Selecting an available model must succeed and switch the active model.
    QVERIFY(protocol->selectModel("model-b"));
    QCOMPARE(protocol->activeModel(), QString("model-b"));

    // Selecting an unknown model must fail.
    QVERIFY(!protocol->selectModel("model-x"));
    QCOMPARE(protocol->activeModel(), QString("model-b"));
}

void NetworkProtocolTest::protocolConnection() {
    // The base class defaults: connect() reports success, the protocol stays
    // disconnected.
    MinimalProtocol minimal;
    QVERIFY(minimal.connect().isSuccessful());
    QVERIFY(!minimal.isConnected());
    QVERIFY(minimal.disconnect().isSuccessful());
    QVERIFY(!minimal.isConnected());

    auto protocol = QSharedPointer<MrSpawner::INetworkProtocol>(new DummyProtocol("llama_cpp", MrSpawner::ProtocolType::LlamaCpp));
    QVERIFY(!protocol->isConnected());
    QVERIFY(protocol->connect().isSuccessful());
    QVERIFY(protocol->isConnected());
    QVERIFY(protocol->disconnect().isSuccessful());
    QVERIFY(!protocol->isConnected());
}

void NetworkProtocolTest::protocolSendDefaults() {
    // The default send() runs query() and announces the result through the
    // finished signal, streamed chunks through the response signal.
    auto protocol = QSharedPointer<MrSpawner::INetworkProtocol>(new DummyProtocol("gpt", MrSpawner::ProtocolType::Gpt));
    QSignalSpy finishedSpy(protocol.data(), &MrSpawner::INetworkProtocol::finished);
    QSignalSpy responseSpy(protocol.data(), &MrSpawner::INetworkProtocol::response);
    QVERIFY(finishedSpy.isValid());
    QVERIFY(responseSpy.isValid());

    MrSpawner::ToolRequest request;
    request.tool = protocol->name();
    QJsonObject parameters;
    parameters.insert("prompt", "stream me");
    request.parameters = parameters;
    MrSpawner::ToolResult result = protocol->send(request);
    QVERIFY(result.isSuccessful());
    QCOMPARE(result.output, QString("gpt:stream me"));
    QCOMPARE(responseSpy.count(), 2);
    QCOMPARE(responseSpy.at(0).at(0).toString(), QString("gpt:"));
    QCOMPARE(responseSpy.at(1).at(0).toString(), QString("stream me"));
    QCOMPARE(finishedSpy.count(), 1);
    const auto finishedResult = finishedSpy.at(0).at(0).value<MrSpawner::ToolResult>();
    QCOMPARE(finishedResult.output, QString("gpt:stream me"));
    QVERIFY(finishedResult.isSuccessful());
}

void NetworkProtocolTest::protocolManagerRegistry() {
    DummyProtocolManager manager;
    QVERIFY(manager.protocols().isEmpty());
    QVERIFY(!manager.hasProtocol("gpt"));
    QVERIFY(!manager.protocol("gpt"));
    QVERIFY(!manager.defaultProtocol());

    auto gpt = QSharedPointer<MrSpawner::INetworkProtocol>(new DummyProtocol("gpt", MrSpawner::ProtocolType::Gpt));
    auto llama = QSharedPointer<MrSpawner::INetworkProtocol>(new DummyProtocol("llama_cpp", MrSpawner::ProtocolType::LlamaCpp));
    QVERIFY(manager.registerProtocol(gpt));
    QVERIFY(manager.registerProtocol(llama));
    QVERIFY(manager.hasProtocol("gpt"));
    QVERIFY(manager.hasProtocol("llama_cpp"));
    QCOMPARE(manager.protocols().size(), 2);
    QCOMPARE(manager.protocol("gpt")->type(), MrSpawner::ProtocolType::Gpt);

    // A duplicate registration must be rejected.
    auto duplicate = QSharedPointer<MrSpawner::INetworkProtocol>(new DummyProtocol("gpt", MrSpawner::ProtocolType::Gpt));
    QVERIFY(!manager.registerProtocol(duplicate));
    QCOMPARE(manager.protocols().size(), 2);

    // The default protocol must be a registered protocol.
    QVERIFY(!manager.setDefaultProtocol("unknown"));
    QVERIFY(manager.setDefaultProtocol("gpt"));
    QVERIFY(manager.defaultProtocol());
    QCOMPARE(manager.defaultProtocol()->name(), QString("gpt"));

    // Unregistering the default protocol must clear the default.
    QVERIFY(manager.unregisterProtocol("gpt"));
    QVERIFY(!manager.defaultProtocol());
    QVERIFY(!manager.hasProtocol("gpt"));

    // Unregistering an unknown protocol must fail.
    QVERIFY(!manager.unregisterProtocol("gpt"));
}

void NetworkProtocolTest::protocolManagerQueries() {
    DummyProtocolManager manager;

    // A query without a default protocol must report an error.
    MrSpawner::ToolRequest request;
    request.tool = "gpt";
    QJsonObject parameters;
    parameters.insert("prompt", "manager query");
    request.parameters = parameters;
    MrSpawner::ToolResult noDefault = manager.query(request);
    QVERIFY(!noDefault.isSuccessful());
    QCOMPARE(static_cast<int>(noDefault.status), static_cast<int>(MrSpawner::ToolStatus::Error));
    MrSpawner::ToolResult noDefaultSend = manager.send(request);
    QVERIFY(!noDefaultSend.isSuccessful());

    // The query must be routed through the default protocol.
    auto gpt = QSharedPointer<MrSpawner::INetworkProtocol>(new DummyProtocol("gpt", MrSpawner::ProtocolType::Gpt));
    QVERIFY(manager.registerProtocol(gpt));
    QVERIFY(manager.setDefaultProtocol("gpt"));
    MrSpawner::ToolResult result = manager.query(request);
    QVERIFY(result.isSuccessful());
    QCOMPARE(result.output, QString("gpt:manager query"));

    QSignalSpy finishedSpy(gpt.data(), &MrSpawner::INetworkProtocol::finished);
    QVERIFY(finishedSpy.isValid());
    MrSpawner::ToolResult sent = manager.send(request);
    QVERIFY(sent.isSuccessful());
    QCOMPARE(sent.output, QString("gpt:manager query"));
    QCOMPARE(finishedSpy.count(), 1);

    // A request without the prompt must produce an error result of the protocol.
    MrSpawner::ToolRequest emptyRequest;
    emptyRequest.tool = "gpt";
    MrSpawner::ToolResult errorResult = manager.query(emptyRequest);
    QVERIFY(!errorResult.isSuccessful());
    QCOMPARE(static_cast<int>(errorResult.status), static_cast<int>(MrSpawner::ToolStatus::Error));
    QVERIFY(!errorResult.error.isEmpty());
}

void NetworkProtocolTest::protocolSignals() {
    // The protocol must report its progress through the QObject signals.
    auto protocol = QSharedPointer<MrSpawner::INetworkProtocol>(new DummyProtocol("gpt", MrSpawner::ProtocolType::Gpt));
    QSignalSpy progressSpy(protocol.data(), &MrSpawner::INetworkProtocol::progress);
    QSignalSpy errorSpy(protocol.data(), &MrSpawner::INetworkProtocol::connectionError);
    QVERIFY(progressSpy.isValid());
    QVERIFY(errorSpy.isValid());

    MrSpawner::ToolRequest request;
    request.tool = protocol->name();
    QJsonObject parameters;
    parameters.insert("prompt", "spy");
    request.parameters = parameters;
    protocol->query(request);
    QCOMPARE(progressSpy.count(), 1);
    QCOMPARE(progressSpy.at(0).at(0).toString(), QString("querying gpt"));

    static_cast<DummyProtocol *>(protocol.data())->raiseConnectionError();
    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(errorSpy.at(0).at(0).toString(), QString("dummy connection error"));

    // The manager must announce registry changes through the QObject signals.
    DummyProtocolManager manager;
    QSignalSpy registeredSpy(&manager, &MrSpawner::INetworkProtocolManager::protocolRegistered);
    QSignalSpy unregisteredSpy(&manager, &MrSpawner::INetworkProtocolManager::protocolUnregistered);
    QSignalSpy defaultSpy(&manager, &MrSpawner::INetworkProtocolManager::defaultProtocolChanged);
    QVERIFY(registeredSpy.isValid());
    QVERIFY(unregisteredSpy.isValid());
    QVERIFY(defaultSpy.isValid());

    QVERIFY(manager.registerProtocol(QSharedPointer<MrSpawner::INetworkProtocol>(new DummyProtocol("gemini", MrSpawner::ProtocolType::Gemini))));
    QCOMPARE(registeredSpy.count(), 1);
    QCOMPARE(registeredSpy.at(0).at(0).toString(), QString("gemini"));

    QVERIFY(manager.setDefaultProtocol("gemini"));
    QCOMPARE(defaultSpy.count(), 1);
    QCOMPARE(defaultSpy.at(0).at(0).toString(), QString("gemini"));

    QVERIFY(manager.unregisterProtocol("gemini"));
    QCOMPARE(unregisteredSpy.count(), 1);
    QCOMPARE(unregisteredSpy.at(0).at(0).toString(), QString("gemini"));
    QCOMPARE(defaultSpy.count(), 2);
    QCOMPARE(defaultSpy.at(1).at(0).toString(), QString(""));
}

// Standard stand-alone QTest entry point, see the Qt test tutorial:
// https://doc.qt.io/qt-6/qttestlib-tutorial1-example.html
QTEST_GUILESS_MAIN(NetworkProtocolTest)

