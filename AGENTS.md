# AGENTS.md

Working agreement for AI agents and human contributors in this repository.

## Short description

Mr.Spawner is a light and fast AI agent written in modern C++ (C++23). It is
built with CMake (3.19+) and split into small, dependency-ordered modules. The
`Core`, `Interfaces` and `mrspawner` modules work without Qt; only the `tests`
module needs Qt6 (QtTest). The project is licensed under GPLv3, see the
[LICENSE](LICENSE) file.

## Architecture and structure

Dependency order (a module only depends on the modules below it):

```
mrspawner (CLI executable, the agent entry point)
    ├── Core (static library, all project functionality, no Qt)
    │       └── Interfaces (static library, public interfaces only, no dependencies)
    └── Interfaces
tests (QtTest executables)
    └── Core  (+ Qt::Core, Qt::Test)
```

Folder layout:

| Path | Description |
|------|-------------|
| `CMakeLists.txt` | Root project. Defines the `MRSPAWNER_TESTS` option and the version string `0.<commit-count>.<commit-hash>` (generated from git history via `updateGitVarsWithHash`), includes the QuasarApp CMake helpers from the submodule and calls `initAll()`. |
| `src/Interfaces/` | Super lightweight static library with all public project interfaces (e.g. `Interfaces/IAgent.h`). No Qt, no Core. Public headers live in `src/public/`. |
| `src/Core/` | Static library that implements all project functions. **Must work without Qt.** Public API in `src/public/Core/` (`core.h`; `global.h` is generated from `global.h.in` at configure time), private code in `src/private/`. |
| `src/mrspawner/` | The `mrspawner` CLI tool for running the agent. Depends on all modules except tests. `deploy/mrspawner.json.in` configures the CQtDeployer distribution package. |
| `tests/` | QtTest-based test suite. One `<name>test.h` + `<name>test.cpp` pair per test class; each pair builds the `MrSpawner-tests-<Name>` executable. |
| `submodules/CMake/` | Git submodule (QuasarApp/CMake). Provides the `test`, `doc`, `deploy` and `release` CI targets and the deploy/version helpers used by `initAll()`. |
| `.github/workflows/` | `pr.yml` — PR build + test matrix (Windows, Linux, macOS with Qt 6.10.3). `cmake-multi-platform-build.yml` — release builds and distribution uploads on `v*` tags. |
| `doxygen.conf.in` | Doxygen configuration template used by the `doc` target. |

Design rules:

* `Interfaces` has zero dependencies; `Core` must never link against Qt.
* Public headers belong to `src/public/`; anything private stays out of it.
* Generated files (`Core/global.h`, `mrspawner.json`, `doxygen.conf`, the
  `build/`, `Testing/` and `Distro/` directories) are produced at
  configure/build time and are git-ignored — never commit them.
* Every source file carries the GPLv3 header comment used by the existing files.

## Contributing rules

* Branch from `main`; do not commit directly to `main`.
* Keep `Core` Qt-free and `Interfaces` dependency-free — new functionality
  belongs in `Core`, new public contracts in `Interfaces`.
* Add or extend tests in `tests/` for new functionality; new test classes
  need an entry in `TEST_CLASSES` in `tests/CMakeLists.txt`.
* Follow the existing C++23 style (4-space indentation, `MrSpawner` namespace)
  and the GPLv3 file header.
* Do not edit the `submodules/CMake` submodule — changes to build helpers
  belong to the QuasarApp/CMake repository.
* A pull request is only mergeable when CI (build + tests on Windows, Linux
  and macOS) is green.

## Build instructions

```sh
# 1. Get the CMake helper submodule (done automatically on fresh clones with --recurse-submodules)
git submodule update --init --recursive

# 2. Configure (Qt6 only needed for the tests module)
cmake -B build -DCMAKE_PREFIX_PATH=<path-to-Qt6> -DCMAKE_BUILD_TYPE=Release

# 3. Build
cmake --build build --config Release
```

To build without the tests module (no Qt required):

```sh
cmake -B build -DMRSPAWNER_TESTS=OFF
cmake --build build
```

Custom targets created by the QuasarApp CMake helpers (`initAll()`):

| Target | Description |
|--------|-------------|
| `make test` | Run the project test suite (needs QtTest and CQtDeployer) |
| `make doc` | Generate the Doxygen documentation (needs doxygen) |
| `make deploy` | Generate the distribution packages (needs CQtDeployer) |
| `make release` | Prepare the release of the project (needs CQtDeployer) |

## Testing instructions

Prerequisites: Qt6 (`Core` and `Test` components), the `cqtdeployer`
executable in `PATH`, and a reachable `qmake` of the Qt installation. The
`test` targets deploy the test executables with CQtDeployer before running
them; CQtDeployer receives the qmake path via the `QT_QMAKE_EXECUTABLE`
variable, which the CI sets through the Qt installation action. With a
system Qt (e.g. distro packages) pass it explicitly at configure time,
e.g. `-DQT_QMAKE_EXECUTABLE=$(which qmake6)`.

```sh
# Configure with the tests enabled (default) and Qt
cmake -B build -DCMAKE_PREFIX_PATH=<path-to-Qt6> -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Run the whole test suite
cmake --build build --target test

# Run a single test class (one target per entry in TEST_CLASSES)
cmake --build build --target testCore
```

* Each `test<Name>` target deploys `MrSpawner-tests-<Name>` into
  `Testing/<name>/` (in the source tree) and executes it there, so the
  executables can also be run manually, e.g. `./Testing/Core/MrSpawner-tests-Core`.
* QtTest writes a text report to `Testing/<name>/test_report.txt`; CI uploads
  these files as the `unit-test-reports-*` artifacts.
* The same commands are used by the CI (`.github/workflows/pr.yml`), where
  `QT_ROOT_DIR` points to the installed Qt.
