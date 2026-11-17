#
# Copyright (C) 2020-2026 QuasarApp.
# Distributed under the GPLv3 software license, see the accompanying
# Everyone is permitted to copy and distribute verbatim copies
# of this license document, but changing it is not allowed.
#

cmake_minimum_required(VERSION 3.19)

# The Interfaces module is a super lightweight library. It contains all the
# public interfaces of the project. It has no project dependencies and is
# built against Qt Core, the base dependency of the project.

set(CURRENT_PROJECT "Interfaces")

file(GLOB_RECURSE SOURCE_CPP
    "src/*.cpp"
    "src/*.h"
)

set(PUBLIC_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/public")

add_library(${CURRENT_PROJECT} STATIC ${SOURCE_CPP})

target_link_libraries(${CURRENT_PROJECT} PUBLIC Qt${QT_VERSION_MAJOR}::Core)

target_include_directories(${CURRENT_PROJECT} PUBLIC ${PUBLIC_INCLUDE_DIR})
