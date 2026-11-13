# Mr.Spawner

Light and fast AI agent written on modern C++.

## Project structure

| Module | Description |
|--------|-------------|
| **Core** (`src/Core`) | The main implementation of all project functions. Works without Qt. |
| **Interfaces** (`src/Interfaces`) | A super lightweight library that contains all public interfaces of the project. |
| **mrspawner** (`src/mrspawner`) | A CLI tool for running the agent. Depends on all modules except tests. |
| **tests** (`tests`) | The testing tool based on QtTest. Depends on Core and Qt. |

## Building

The project is builded with CMake (3.19+). Qt6 is required only for the
tests module, the `Core`, `Interfaces` and `mrspawner` modules work without Qt.

```sh
cmake -B build -DCMAKE_PREFIX_PATH=<path-to-Qt6> -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

To build without the tests module (no Qt required):

```sh
cmake -B build -DMRSPAWNER_TESTS=OFF
cmake --build build
```

## Build targets

|   Command or make target   |  Description    |
|------|------|
| **make test** | The run tests for a project (dependent of QtTest and CQtDeployer) |
| **make doc** | The generate a documentation for a project (dependent of doxygen) |
| **make deploy** | The generate distribution for a project (dependent of CQtDeployer) |
| **make release** | The prepare the release of a project (dependent of CQtDeployer) |

## CI

* `pr.yml` - pull request test action for Windows, Linux and macOS.
* `cmake-multi-platform-build.yml` - release build action, creates a GitHub
  release with the distribution packages on the `v*` tags.

## License

GPLv3 - see the [LICENSE](LICENSE) file.

