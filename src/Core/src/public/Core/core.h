//
// Copyright (C) 2020-2026 QuasarApp.
// Distributed under the GPLv3 software license, see the accompanying
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//

#include "Core/global.h"

#include <string>

namespace MrSpawner {

/**
 * @brief init main initialize method of The Core library
 * @return true if library initialized successfull
 */
CORE_EXPORT bool init();

/**
 * @brief version This method return string value of a library version
 * @return string value of a library version
 */
CORE_EXPORT std::string version();

};
