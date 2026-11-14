#
# Copyright (C) 2020-2026 QuasarApp.
# Distributed under the GPLv3 software license, see the accompanying
# Everyone is permitted to copy and distribute verbatim copies
# of this license document, but changing it is not allowed.
#

cmake_minimum_required(VERSION 3.19)
project(MrSpawner LANGUAGES CXX)
if(TARGET ${PROJECT_NAME})
  message("The ${PROJECT_NAME} already included in main Project")
  return()
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(MRSPAWNER_TESTS "This option disables or enables tests of the ${PROJECT_NAME} project"  ON)

if (ANDROID OR IOS OR QA_WASM32)
    set(MRSPAWNER_TESTS OFF CACHE BOOL "This option force disabled for ANDROID IOS QA_WASM32 and Not Qt projects" FORCE)
endif()

# Qt is only required by the tests module. The Core, Interfaces and
# mrspawner modules work without Qt. The versionless QT package name sets
# the QT_VERSION_MAJOR variable, so the Qt${QT_VERSION_MAJOR}:: targets can
# be used in the modules.
if (MRSPAWNER_TESTS)
    find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Test QUIET)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)

    # Do not use qt_standard_project_setup() here: on Windows it sets
    # CMAKE_RUNTIME_OUTPUT_DIRECTORY to the directory where the macro is
    # invoked (the build root), so every executable leaves its target
    # directory. The CI test targets and the deploy configuration expect
    # the executables in their target directories (${CMAKE_BINARY_DIR}/tests
    # and ${CMAKE_BINARY_DIR}/src/mrspawner), which is the CMake default.
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
endif()

include(submodules/CMake/QuasarApp.cmake)

updateGitVarsWithHash("f1ede5ed149fe706e1426e2c76c8fc6172bf349c")

set(MRSPAWNER_VERSION "0.${GIT_COMMIT_COUNT}.${GIT_COMMIT_HASH}")

set(MRSPAWNER_PACKAGE_ID "quasarapp.core.MrSpawner")

initAll()

add_subdirectory(src/Interfaces)
add_subdirectory(src/Core)
add_subdirectory(src/mrspawner)

if (MRSPAWNER_TESTS)
    add_subdirectory(tests)
else()
    message("The ${PROJECT_NAME} tests is disabled.")
endif()

configure_file_in(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/doxygen.conf)
addDoc(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/doxygen.conf)
