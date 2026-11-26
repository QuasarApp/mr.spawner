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

# Qt Core is the base dependency of the project: the Interfaces, Core and
# mrspawner modules are built against it, and the tests module additionally
# needs the Qt Test component. The versionless QT package name sets the
# QT_VERSION_MAJOR variable, so the Qt${QT_VERSION_MAJOR}:: targets can be
# used in the modules.
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core QUIET)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

if (MRSPAWNER_TESTS)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)
endif()

# The QuasarAppLib test suite is built only when the QuasarAppLib is the
# top-level project: the QuasarAppTests CMakeLists resolves the library
# include directories with the CMAKE_SOURCE_DIR variable, so the embedded
# build cannot compile the suite. The suite is covered by the QuasarAppLib
# own CI, so it is disabled here.
set(QA_TESTS OFF CACHE BOOL "Enable or disable tests of the QuasarApp library" FORCE)

# Do not use qt_standard_project_setup() here: on Windows it sets
# CMAKE_RUNTIME_OUTPUT_DIRECTORY to the directory where the macro is
# invoked (the build root), so every executable leaves its target
# directory. The CI test targets and the deploy configuration expect
# the executables in their target directories (${CMAKE_BINARY_DIR}/tests
# and ${CMAKE_BINARY_DIR}/src/mrspawner), which is the CMake default.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# The QuasarApp CMake layer (the CI targets and the deploy/version helpers)
# is provided by the QuasarAppLib submodule, the cmake include is internal
# to the library (submodules/QuasarAppLib/CMake/QuasarApp.cmake).
include(submodules/QuasarAppLib/CMake/QuasarApp.cmake)

updateGitVarsWithHash("f1ede5ed149fe706e1426e2c76c8fc6172bf349c")

set(MRSPAWNER_VERSION "0.${GIT_COMMIT_COUNT}.${GIT_COMMIT_HASH}")

set(MRSPAWNER_PACKAGE_ID "quasarapp.core.MrSpawner")

initAll()

add_subdirectory(submodules/QuasarAppLib)

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
