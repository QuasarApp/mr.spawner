#
# Copyright (C) 2020-2026 QuasarApp.
# Distributed under the GPLv3 software license, see the accompanying
# Everyone is permitted to copy and distribute verbatim copies
# of this license document, but changing it is not allowed.
#

cmake_minimum_required(VERSION 3.19)

# The Interfaces module is a super lightweight library. It contains all the
# public interfaces of the project and has no dependencies (no Qt, no Core).

set(CURRENT_PROJECT "Interfaces")

file(GLOB_RECURSE SOURCE_CPP
    "src/*.cpp"
    "src/*.h"
)

set(PUBLIC_INCUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/public")

add_library(${CURRENT_PROJECT} STATIC ${SOURCE_CPP})

target_include_directories(${CURRENT_PROJECT} PUBLIC ${PUBLIC_INCUDE_DIR})
