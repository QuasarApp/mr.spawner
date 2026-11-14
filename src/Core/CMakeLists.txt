#
# Copyright (C) 2020-2026 QuasarApp.
# Distributed under the GPLv3 software license, see the accompanying
# Everyone is permitted to copy and distribute verbatim copies
# of this license document, but changing it is not allowed.
#

cmake_minimum_required(VERSION 3.19)

# The Core module is the main implementation of all project functions.
# It must work without Qt.

set(CURRENT_PROJECT "Core")
add_definitions(-DCORE_LIBRARY)

file(GLOB_RECURSE SOURCE_CPP
    "src/*.cpp"
    "src/*.h"
)

set(PUBLIC_INCUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/public")
set(PRIVATE_INCUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/private")

add_library(${CURRENT_PROJECT} STATIC ${SOURCE_CPP})

# Core implements the public interfaces of the project.
target_link_libraries(${CURRENT_PROJECT} PUBLIC Interfaces)

target_include_directories(${CURRENT_PROJECT} PUBLIC ${PUBLIC_INCUDE_DIR})
target_include_directories(${CURRENT_PROJECT} PRIVATE ${PRIVATE_INCUDE_DIR})

configure_file_in(${CURRENT_PROJECT} "${CMAKE_CURRENT_SOURCE_DIR}/src/public/Core/global.h")