.. _sessions:

Sessions
========

UDS operates through diagnostic sessions, which are states in which the ECU operates to
perform specific diagnostic or service functions.
Each session provides a different level of access to the ECU's functionality.
The ECU starts in a default session, but diagnostic tools can request transitions
to other sessions to perform specialized operations.

Types of UDS Sessions

1. Default Session (0x01), class ``ApplicationDefaultSession``.

   - **Description:** This is the basic session active upon ECU startup.
   - **Functionality:** Limited diagnostic services are available, primarily for querying information (e.g., ID requests, DTCs).
   - **Typical Use Cases:** Basic diagnostics, reading fault codes.

2. Programming Session (0x02), class ``ProgrammingSession``.

   - **Description:** Used for reprogramming the ECU firmware.
   - **Functionality:** Enables downloading new software or calibration data into the ECU.
   - **Typical Use Cases:** ECU flashing, firmware updates.

3. Extended Diagnostic Session (0x03), class ``ApplicationExtendedSession``.

   - **Description:** Provides enhanced diagnostic functionality beyond the default session.
   - **Functionality:** Includes advanced capabilities like actuator testing, detailed fault code analysis, and special function testing.
   - **Typical Use Cases:** In-depth diagnostics, calibration, and testing.
