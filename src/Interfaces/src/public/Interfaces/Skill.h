//
// Copyright (C) 2026-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#ifndef INTERFACES_SKILL_H
#define INTERFACES_SKILL_H

#include "Interfaces/ToolRequest.h"
#include "Interfaces/ToolResult.h"

#include <QObject>
#include <QString>
#include <QJsonObject>

namespace MrSpawner {

/**
 * @brief The SkillCategory enum
 *
 * Classifies skills by the type of capability they provide. The category
 * is used by the agent to select, group or restrict skills (e.g. only
 * search skills for analysis requests).
 */
enum class SkillCategory {
    Unknown = 0,      //!< The skill does not fit into a specific category
    Search,           //!< Search and analysis capabilities
    Write,            //!< Content creation and modification
    Read,             //!< Data retrieval and reading
    Execute,          //!< Command execution and process control
    Agent,            //!< Agent coordination and delegation
    Knowledge,        //!< Knowledge retrieval and reasoning
    Tool,             //!< Tool orchestration and integration
};

/**
 * @brief The Skill interface
 *
 * The base interface of all skills that the AI agent can leverage. A skill
 * represents a reusable capability that extends the agent's functionality.
 * Skills are described by their name, description, and the context they
 * operate within (see the skill protocol of the agent framework).
 *
 * Skill is a QObject, the Qt base class of the project: long-running
 * skills report their progress to the agent through the progress signal.
 */
class Skill : public QObject {
    Q_OBJECT
public:
    virtual ~Skill() = default;

    /**
     * @brief name the unique name of the skill in the skill protocol
     * @return the skill name, e.g. "code_analysis"
     */
    virtual QString name() const = 0;

    /**
     * @brief displayName a short human readable name of the skill
     * @return the display name, defaults to name()
     */
    virtual QString displayName() const {
        return name();
    }

    /**
     * @brief description the description of the skill that is shown to the AI model
     * @return the skill description
     */
    virtual QString description() const = 0;

    /**
     * @brief contextDescription the JSON description of the context/skills this skill provides
     * @return the JSON schema of the context
     */
    virtual QString contextDescription() const = 0;

    /**
     * @brief category the category of the skill capability
     * @return the skill category, defaults to SkillCategory::Unknown
     */
    virtual SkillCategory category() const {
        return SkillCategory::Unknown;
    }

    /**
     * @brief requiresApproval true if the skill activation needs user approval before use
     * @return true when the agent must ask the user before activating the skill
     */
    virtual bool requiresApproval() const {
        return false;
    }

    /**
     * @brief activate activates the skill for use by the agent
     * @param context the context/parameters for activating the skill
     * @return the activation result with any context information
     */
    virtual ToolResult activate(const ToolRequest &context) = 0;

    /**
     * @brief deactivate deactivates the skill
     * @return the deactivation result
     */
    virtual ToolResult deactivate() {
        return ToolResult();
    }

    /**
     * @brief isActive true if the skill is currently active
     * @return true when the skill is active
     */
    virtual bool isActive() const {
        return false;
    }

    /**
     * @brief abort stops an in-flight skill activation/deactivation.
     * The default implementation does nothing; skills that run processes
     * or network requests override it to stop the work. The method must
     * be safe to call before activation started, after it finished
     * and multiple times.
     */
    virtual void abort() {
    }

signals:
    /**
     * @brief progress reports a progress message of an in-flight activation
     * @param message the progress message, e.g. the current step of skill activation
     */
    void progress(const QString &message);

protected:
    explicit Skill(QObject *parent = nullptr) : QObject(parent) {
    }
};

}

#endif // INTERFACES_SKILL_H