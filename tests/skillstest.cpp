//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#include "skillstest.h"

#include "Interfaces/Skill.h"
#include "Interfaces/IToolSkillManager.h"

#include <QHash>
#include <QJsonObject>
#include <QSignalSpy>
#include <QSharedPointer>
#include <QString>
#include <QStringList>

namespace {

class DummySkill : public MrSpawner::Skill {
public:
    explicit DummySkill(QObject *parent = nullptr) : MrSpawner::Skill(parent) {
    }

    QString name() const override {
        return QStringLiteral("dummy_skill");
    }

    QString description() const override {
        return QStringLiteral("A dummy skill that echoes the context parameter");
    }

    QString contextDescription() const override {
        return QStringLiteral(R"({ "type": "object", "properties": { "input": { "type": "string" } } })");
    }

    MrSpawner::SkillCategory category() const override {
        return MrSpawner::SkillCategory::Unknown;
    }

    MrSpawner::ToolResult activate(const MrSpawner::ToolRequest &context) override {
        emit progress(QStringLiteral("activating skill"));
        MrSpawner::ToolResult result;
        if (!context.parameters.contains(QStringLiteral("input"))) {
            result.status = MrSpawner::ToolStatus::Error;
            result.error = QStringLiteral("the input parameter is missing");
            return result;
        }
        result.output = QStringLiteral("dummy_skill:") + context.parameters.value(QStringLiteral("input")).toString();
        result.status = MrSpawner::ToolStatus::Success;
        return result;
    }
};

class DummySkillManager : public MrSpawner::IToolSkillManager {
    QHash<QString, QSharedPointer<MrSpawner::Skill>> m_skills;
public:
    explicit DummySkillManager(QObject *parent = nullptr) : MrSpawner::IToolSkillManager(parent) {
    }

    bool registerSkill(const QSharedPointer<MrSpawner::Skill> &skill) override {
        if (m_skills.contains(skill->name())) {
            return false;
        }
        m_skills.insert(skill->name(), skill);
        emit skillRegistered(skill->name());
        return true;
    }

    bool unregisterSkill(const QString &skillName) override {
        if (m_skills.remove(skillName) == 0) {
            return false;
        }
        emit skillUnregistered(skillName);
        return true;
    }

    bool hasSkill(const QString &skillName) const override {
        return m_skills.contains(skillName);
    }

    QSharedPointer<MrSpawner::Skill> skill(const QString &skillName) const override {
        return m_skills.value(skillName);
    }

    QStringList skills() const override {
        return m_skills.keys();
    }

    MrSpawner::ToolResult execute(const MrSpawner::ToolRequest &request) override {
        auto it = m_skills.find(request.tool);
        if (it == m_skills.end()) {
            MrSpawner::ToolResult result;
            result.status = MrSpawner::ToolStatus::Error;
            result.error = QStringLiteral("unknown skill: ") + request.tool;
            return result;
        }
        return it->data()->activate(request);
    }
};

} // namespace

void SkillsTest::initTestCase() {
}

void SkillsTest::cleanupTestCase() {
}

void SkillsTest::skillDefaults() {
    DummySkill skill;

    QCOMPARE(skill.name(), QString("dummy_skill"));
    QVERIFY(!skill.name().isEmpty());
    QCOMPARE(skill.description(), QString("A dummy skill that echoes the context parameter"));
    QVERIFY(!skill.description().isEmpty());
    QCOMPARE(skill.contextDescription(), QString(R"({ "type": "object", "properties": { "input": { "type": "string" } } })"));
    QVERIFY(!skill.contextDescription().isEmpty());
    QCOMPARE(skill.displayName(), QString("dummy_skill"));
    QCOMPARE(skill.category(), MrSpawner::SkillCategory::Unknown);
    QCOMPARE(skill.requiresApproval(), false);
    QCOMPARE(skill.isActive(), false);
    MrSpawner::ToolResult deactivation = skill.deactivate();
    QCOMPARE(deactivation.status, MrSpawner::ToolStatus::Success);
}

void SkillsTest::skillManagerProtocol() {
    DummySkillManager manager;
    QVERIFY(manager.skills().isEmpty());
    QVERIFY(!manager.hasSkill("dummy_skill"));
    QVERIFY(!manager.skill("dummy_skill"));

    auto skill = QSharedPointer<MrSpawner::Skill>(new DummySkill);
    QVERIFY(manager.registerSkill(skill));
    QVERIFY(manager.hasSkill("dummy_skill"));
    QCOMPARE(manager.skills().size(), 1);
    QCOMPARE(manager.skill("dummy_skill")->name(), QString("dummy_skill"));

    auto duplicate = QSharedPointer<MrSpawner::Skill>(new DummySkill);
    QVERIFY(!manager.registerSkill(duplicate));

    MrSpawner::ToolRequest request;
    request.tool = "dummy_skill";
    QJsonObject parameters;
    parameters.insert(QStringLiteral("input"), QStringLiteral("manager"));
    request.parameters = parameters;
    MrSpawner::ToolResult result = manager.execute(request);
    QVERIFY(result.isSuccessful());
    QCOMPARE(result.output, QString("dummy_skill:manager"));

    MrSpawner::ToolRequest unknown;
    unknown.tool = "unknown_skill";
    MrSpawner::ToolResult errorResult = manager.execute(unknown);
    QVERIFY(!errorResult.isSuccessful());
    QCOMPARE(static_cast<int>(errorResult.status), static_cast<int>(MrSpawner::ToolStatus::Error));
    QVERIFY(!errorResult.error.isEmpty());

    QVERIFY(manager.unregisterSkill("dummy_skill"));
    QVERIFY(!manager.unregisterSkill("dummy_skill"));
    QVERIFY(!manager.hasSkill("dummy_skill"));
}

void SkillsTest::skillSignals() {
    auto skill = QSharedPointer<MrSpawner::Skill>(new DummySkill);
    QSignalSpy progressSpy(skill.data(), &MrSpawner::Skill::progress);
    QVERIFY(progressSpy.isValid());

    MrSpawner::ToolRequest request;
    request.tool = skill->name();
    QJsonObject parameters;
    parameters.insert(QStringLiteral("input"), QStringLiteral("spy"));
    request.parameters = parameters;
    skill->activate(request);
    QCOMPARE(progressSpy.count(), 1);
    QCOMPARE(progressSpy.at(0).at(0).toString(), QString("activating skill"));

    DummySkillManager manager;
    QSignalSpy registeredSpy(&manager, &MrSpawner::IToolSkillManager::skillRegistered);
    QSignalSpy unregisteredSpy(&manager, &MrSpawner::IToolSkillManager::skillUnregistered);
    QVERIFY(registeredSpy.isValid());
    QVERIFY(unregisteredSpy.isValid());

    QVERIFY(manager.registerSkill(QSharedPointer<MrSpawner::Skill>(new DummySkill)));
    QCOMPARE(registeredSpy.count(), 1);
    QCOMPARE(registeredSpy.at(0).at(0).toString(), QString("dummy_skill"));

    QVERIFY(manager.unregisterSkill("dummy_skill"));
    QCOMPARE(unregisteredSpy.count(), 1);
    QCOMPARE(unregisteredSpy.at(0).at(0).toString(), QString("dummy_skill"));
}

// Standard stand-alone QTest entry point
QTEST_GUILESS_MAIN(SkillsTest)