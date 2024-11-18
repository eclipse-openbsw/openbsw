# Eclipse OpenBSW

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

## Media

Talks:

[Eclipse OpenBSW - A fresh view on Automotive Microcontroller Software - OCX 2024](https://www.youtube.com/watch?v=9zlya-cdrgo)

Articles:

[OpenBSW: A Code-first Software Platform for Automotive Microcontrollers](https://medium.com/@ESRLabs/openbsw-a-code-first-software-platform-for-automotive-microcontrollers-609d3406cf0d)

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
| UDP, DoCAN | Diagnostics over CAN | If ``SocketCAN`` is supported | Yes | Since Release 0.1 |

## Roadmap

| Feature | Description | Planned for |
| --- | --- | --- |
| Ethernet | Add support for Ethernet communication to demo | Release 0.2 |
| DoIP | Add support for Diagnostics over IP to demo | Release 0.2 |

## Documentation

The [documentation](https://eclipse-openbsw.github.io/openbsw)
describes Eclipse OpenBSW in detail and provides simple setup guides to build and use it.

## Contributing

It is expected that this respository will be used as a starting point for many custom developments.
You may wish to contribute back some of your work to this repository.
For more details see [CONTRIBUTING](CONTRIBUTING.md).

## Legals

Distributed under the [Apache 2.0 License](LICENSE).

Also see [NOTICE](NOTICE.md).
