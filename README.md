# Eclipse OpenBSW


## Build Status 🚀

[![Build S32k and posix platform](https://github.com/eclipse-openbsw/openbsw/actions/workflows/build.yml/badge.svg?branch=main&event=push)](https://github.com/eclipse-openbsw/openbsw/actions/workflows/build.yml)

## Code Coverage 

| Code Coverage            | Status                                                                 |
|--------------------------|------------------------------------------------------------------------|
| Line Coverage            | [![Line Coverage](https://eclipse-openbsw.github.io/openbsw/coverage_badges/line_coverage_badge.svg)](https://github.com/eclipse-openbsw/openbsw/actions/workflows/code-coverage.yml) |
| Function Coverage        | [![Function Coverage](https://eclipse-openbsw.github.io/openbsw/coverage_badges/function_coverage_badge.svg)](https://github.com/eclipse-openbsw/openbsw/actions/workflows/code-coverage.yml) |

## Overview

Eclipse OpenBSW is an open source SDK to build professional, high quality embedded software products. 
It is a software stack specifically designed and developed for automotive applications.

This repository provides the complete code, documentation and a reference example
that works out of the box without any specific hardware requirements (any POSIX platform)
allowing developers to get up and running quickly.

## Target Audience

* **Open Source Enthusiasts**: Enthusiasts and hobbyists passionate about automotive technology
  and interested in contributing to open source projects, collaborating with like-minded
  individuals and exploring new ideas and projects in the automotive domain.

* **Embedded Systems Developers**: Developers specializing in embedded systems programming,
  microcontroller firmware development and real-time operating systems (RTOS), who are interested
  in automotive applications.

* **Automotive Engineers**: Professionals working in the automotive industry, including engineers,
  designers and technicians, who are interested in developing and improving automotive
  technologies, systems and components.

* **Students and Researchers**: Students, researchers, and academic institutions interested in
  learning about automotive technologies, conducting research, and exploring innovative solutions
  in automotive areas.

## Getting Started

To get started, we recommend to compile our reference application for one of the supported platforms
using the docker image we provide including all the necessary tools. Therefore, you can simply run
the development service in the docker compose in the root of the repo, call cmake with the correct
options and build the generated project.

> [!NOTE]
> 
> In case your local user already uses `UID`/`GID` of `1000` you can skip the `DOCKER_UID` and
> `DOCKER_GID` variables, since this is the default. Otherwise, you need it to make sure you have
> proper access to your local files.
> 
> In case you want to use a custom history file for the commands you run in the container you can
> also set the `DOCKER_HISTORY` variable, which defaults to the `~/.docker_history` file.
>
> Note, that we bind mount your current working directory into the container and use it as working
> directory. This makes sure, that you will have the same paths inside and outside of the container
> when for example loading a generated elf into a debugger or following symlinks created within the
> container.

```
host> DOCKER_UID=$(id -u) DOCKER_GID=$(id -g) docker compose run development
docker> cmake --preset posix
docker> cmake --build --preset posix
```

## Feature Overview

### Implemented Features

| Feature | Description | POSIX Support | S32K148 Support | New? |
| --- | --- | --- | --- | --- |
| Modular design | Based on each project's needs, required software modules can easily be included or excluded. | Yes | Yes | |
| Application Lifecycle Management | The order in which Applications/Features are brought up/down is easily organised. | Yes | Yes | |
| Console | A console is provided for diagnostic and development purposes. | In a terminal interface | Via UART | |
| Commands | Commands can easily be added to the console to aid development, test and debugging. | Yes | Yes | |
| Logging | Diagnostic logging is implemented per software component. | Yes | Yes | |
| CAN | Support for CAN bus communication | If ``SocketCAN`` is supported | Yes | Since Release 0.1 |
| Sensors and actuators integration | ADC, PWM & GPIO | | Yes | |
| UDS, DoCAN | Diagnostics over CAN | If ``SocketCAN`` is supported | Yes | Since Release 0.1 |
| Ethernet | Basic TCP and UDP support | Yes | Yes | On current `main` |

## Roadmap

See [GitHub Issues](https://github.com/eclipse-openbsw/openbsw/issues?q=is%3Aissue%20state%3Aopen%20label%3Aenhancement).

## Documentation

The [documentation](https://eclipse-openbsw.github.io/openbsw)
describes Eclipse OpenBSW in detail and provides simple setup guides to build and use it.

## Contributing

It is expected that this repository will be used as a starting point for many custom developments.
You may wish to contribute back some of your work to this repository.
For more details see [CONTRIBUTING](CONTRIBUTING.md).

## Legals

Distributed under the [Apache 2.0 License](LICENSE).

Also see [NOTICE](NOTICE.md).
