#
# Copyright (C) 2020-2026 QuasarApp.
# Distributed under the GPLv3 software license, see the accompanying
# Everyone is permitted to copy and distribute verbatim copies
# of this license document, but changing it is not allowed.
#

cmake_minimum_required(VERSION 3.19)

# The tests module is the testing tool based on QtTest.
# It depends on Core and Qt.

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)

# The deployTest targets run cqtdeployer and create the deployment folders
# inside the common ${DIR_FOR_TESTING} directory. If that parent directory
# does not exist yet, the parallel cqtdeployer instances race on its
# creation. Create the directory in a dedicated target that every deploy
# target depends on.
add_custom_target(
    prepareTestDir
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DIR_FOR_TESTING}
    COMMENT "Create the ${DIR_FOR_TESTING} directory for the parallel test deployments")

# The deployTest<Name> targets of the addTestsArg helper pass the
# QT_ROOT_DIR variable to the cqtdeployer libDir option. It must contain
# the Qt installation directory, otherwise the Qt libraries of the test
# executable are classified as system libraries and are not deployed.
set(QT_ROOT_DIR "$ENV{QT_ROOT_DIR}")
message("QT_ROOT_DIR = ${QT_ROOT_DIR}")

set(TEST_CLASSES Core)

foreach (TEST_CLASS ${TEST_CLASSES})
    string(TOLOWER ${TEST_CLASS} TEST_CLASS_LOWER)
    set(TEST_EXEC ${PROJECT_NAME}-tests-${TEST_CLASS})

    add_executable(${TEST_EXEC} ${TEST_CLASS_LOWER}test.cpp ${TEST_CLASS_LOWER}test.h)
    target_link_libraries(${TEST_EXEC} PRIVATE Qt${QT_VERSION_MAJOR}::Test Core)

    # Register the test executable with the QuasarApp CI test targets. The
    # test argument makes QtTest write the test_report.txt report.
    addTestsArg(${TEST_CLASS} ${TEST_EXEC} "-o;test_report.txt;-txt")

    add_dependencies(deployTest${TEST_CLASS} ${TEST_EXEC} prepareTestDir)
endforeach()
