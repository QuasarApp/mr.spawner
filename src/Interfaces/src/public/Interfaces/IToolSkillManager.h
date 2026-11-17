//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef INTERFACES_ITOOLSKILLMANAGER_H
#define INTERFACES_ITOOLSKILLMANAGER_H

#include "Interfaces/Skill.h"

#include <QSharedPointer>
#include <QString>
#include <QStringList>

namespace MrSpawner {

/**
 * @brief The IToolSkillManager interface
 *
 * The central registry and executor of the agent skills. Skills register
 * themselves here and the agent executes skill activations through this
 * interface. The Core module provides the implementation of this
 * interface.
 *
 * IToolSkillManager is a QObject, the Qt base class of the project:
 * skill changes are announced to the agent through the skillRegistered
 * and skillUnregistered signals.
 */
class IToolSkillManager : public QObject {
    Q_OBJECT
public:
    virtual ~IToolSkillManager() = default;

    /**
     * @brief registerSkill adds a skill to the registry
     * @param skill the skill to register
     * @return false when a skill with the same name is already registered
     */
    virtual bool registerSkill(const QSharedPointer<Skill> &skill) = 0;

    /**
     * @brief unregisterSkill removes a skill from the registry
     * @param skillName the name of the skill to remove
     * @return false when no skill with that name is registered
     */
    virtual bool unregisterSkill(const QString &skillName) = 0;

    /**
     * @brief hasSkill true if a skill with the given name is registered
     * @param skillName the name of the skill
     * @return true when the skill is registered
     */
    virtual bool hasSkill(const QString &skillName) const = 0;

    /**
     * @brief skill returns the registered skill
     * @param skillName the name of the skill
     * @return the skill or a null QSharedPointer when the name is unknown
     */
    virtual QSharedPointer<Skill> skill(const QString &skillName) const = 0;

    /**
     * @brief skills the names of all registered skills
     * @return the list of skill names
     */
    virtual QStringList skills() const = 0;

    /**
     * @brief execute activates a skill with the given context
     * @param context the skill activation context/request
     * @return the skill activation result; an Error result when the skill name is unknown
     */
    virtual ToolResult execute(const ToolRequest &context) = 0;

signals:
    /**
     * @brief skillRegistered a new skill was added to the registry
     * @param skillName the name of the registered skill
     */
    void skillRegistered(const QString &skillName);

    /**
     * @brief skillUnregistered a skill was removed from the registry
     * @param skillName the name of the removed skill
     */
    void skillUnregistered(const QString &skillName);

protected:
    explicit IToolSkillManager(QObject *parent = nullptr) : QObject(parent) {
    }
};

}

#endif // INTERFACES_ITOOLSKILLMANAGER_H