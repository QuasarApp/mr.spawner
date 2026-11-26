//
// Copyright (C) 2020-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#include "Core/core.h"

#include "OptionData/optiondata.h"
#include "Params/params.h"
#include "Settings/config.h"
#include "Utils/qalogger.h"

#include <QCoreApplication>

using namespace QuasarAppUtils;

namespace {

/**
 * @brief agentOptions This function returns the list of the command-line
 * options of the mrspawner agent. The base options (the -verbose and the
 * -fileLog options) are added by the Params class automatically, so they
 * are not required here.
 * @return the options list of the mrspawner agent.
 */
OptionsDataList agentOptions() {
    OptionsDataList options;
    options.insert("Mr.Spawner Options",
                   OptionData{{"-agent"}, "(name)", "The name of the agent that must be run."});
    options.insert("Mr.Spawner Options",
                   OptionData{{"-model"}, "(name)", "The name of the AI model that the agent uses."});
    return options;
}

} // namespace

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("MrSpawner");
    QCoreApplication::setOrganizationName("QuasarApp");
    QCoreApplication::setApplicationVersion(QString::fromStdString(MrSpawner::version()));

    // The Config class is the main system of configuration of the
    // mrspawner application. It parses the command line (with the
    // Params class) and resolves the value of a setting with the
    // priority: command line > config file > default value.
    Config config(argc, const_cast<const char**>(argv), agentOptions());

    // The QALogger installs the Qt message handler that writes the
    // messages to the log file (when the -fileLog option is set) and
    // filters them by the -verbose level. It must be initialized after
    // the command line was parsed.
    QALogger logger;
    logger.init();

    if (!MrSpawner::init()) {
        qCritical() << "The MrSpawner core module was not initialized";
        return 1;
    }

    const QString agent = config.getStrValue("agent");
    const QString model = config.getStrValue("model");

    qInfo() << "The mrspawner version" << QCoreApplication::applicationVersion() << "was started";

    if (agent.isEmpty()) {
        qWarning() << "The agent name was not provided (the -agent option). "
                     "The agent was not started.";
    } else {
        qInfo() << "The" << agent << "agent was started with the"
                << (model.isEmpty() ? QStringLiteral("default") : model) << "model.";
    }

    return 0;
}
