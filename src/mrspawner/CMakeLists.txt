#
# Copyright (C) 2020-2026 QuasarApp.
# Distributed under the GPLv3 software license, see the accompanying
# Everyone is permitted to copy and distribute verbatim copies
# of this license document, but changing it is not allowed.
#

cmake_minimum_required(VERSION 3.19)

# The mrspawner module is the CLI tool for running the agent.
# It depends on all project modules except tests.

set(CURRENT_PROJECT "mrspawner")

file(GLOB_RECURSE SOURCE_CPP
    "*.cpp" "*.h"
)

set(PUBLIC_INCUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(${CURRENT_PROJECT} ${SOURCE_CPP})

target_link_libraries(${CURRENT_PROJECT} PUBLIC Core Interfaces)

target_include_directories(${CURRENT_PROJECT} PUBLIC ${PUBLIC_INCUDE_DIR})

# Prepare the distribution package of the mrspawner CLI. The package names
# are exported to the GitHub Actions environment, see the
# .github/workflows/cmake-multi-platform-build.yml release action.

SET(TARGET_DIR "${CMAKE_SOURCE_DIR}/Distro")
file(MAKE_DIRECTORY ${TARGET_DIR})

set(MRSPAWNER_DISTRO_NAME "mrspawner_${MRSPAWNER_VERSION}")
set(MRSPAWNER_DISTRO_FULL_NAME "${MRSPAWNER_DISTRO_NAME}_${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}")

if(DEFINED ENV{GITHUB_ENV})
    file(APPEND $ENV{GITHUB_ENV} "MRSPAWNER_VERSION=${MRSPAWNER_VERSION}\n")
    file(APPEND $ENV{GITHUB_ENV} "MRSPAWNER_DISTRO_FULL_NAME=${MRSPAWNER_DISTRO_FULL_NAME}\n")
endif()

configure_file_in(${CURRENT_PROJECT} "${CMAKE_CURRENT_SOURCE_DIR}/deploy/mrspawner.json")

addDeployFromCustomFile(${CURRENT_PROJECT} "${CMAKE_CURRENT_SOURCE_DIR}/deploy/mrspawner.json")
