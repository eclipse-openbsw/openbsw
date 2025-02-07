# Changelog

All notable changes to this project will be documented in this file.

## [0.2.0] - 2024-11-19

### Added

* Added/updated the documentation for:
    * asyncImpl module
    * timer::Timer
    * async module
    * common
    * asyncConsole
    * stdioConsoleInput
    * Updated the Testing Guide
    * Change location of linker script in setup documentation
    * setup gdb server, Visual Studio Code and IntelliSense
    * building referenceApp unit tests
    * Improve links in io docs
* Add ADC test for channel 1
* Add support in pytest for a hardware testing board
* Generate docs from code using Doxygen
* Add treefmt and improve code formatting doc
* Add ADC channel
* Add VS code settings files
* Add console lifecycle and logger test cases
* Add DemoCommand as AsyncConsole testcase
* Add code coverage in CMakeLists.txt
* Implement pytest for console command stats stack
* Add cmake-format via cmakelang
* Create CMake OpenBSW module

### Changed

* Bulk reformat to comply to our clang-format file
* Make default C and CXX version overridable
* Centralizing Version Information
* Use hardware-based ticks in StatisticsWriter
* Update nxp IDE setup to 3.5
* Replace socat with python alternative
* Unhide toctree in platform doc landing page
* Clean up includes in application/systems
* Switch camel_case to PascalCalse filenames
* Downgrade cmake to version 3.22
* Make unit test separate executable
* Move platform support code into executables
* Switch from platform::atomic to std::atomic

### Fixed

* Minor fixes of console
* Fix pytest failures by adding delays and retries in serial write
* Fix POSIX CanSystem startup
* Increase code coverage of RoutineControlJob.cpp in uds
* Fix reference to multiple cores in directory structure
* Fix minor compilation warnings
* Fix unsupported assembler flag

### Removed

* C++11 support
* platform support options from unitTest
* DCC optimization from big_endian.h
* platform/atomic from referenceApp
* unused scale parameter from io docs

## [0.1.0] - 2024-10-17

### Added

- First public release.
