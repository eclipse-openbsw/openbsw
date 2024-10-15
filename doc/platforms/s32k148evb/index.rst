.. _s32k148evb_overview:

S32K148 Evaluation Board
========================

Overview
--------

- The S32K148 Evaluation Board is a development platform designed by NXP Semiconductors, aimed at
  simplifying the development process for automotive and industrial applications.
- Eclipse OpenBSW provides a reference application for this platform which can be built
  out-of-the-box and flashed onto the evaluation board, enabling the user to test working features
  straight away, thus immediately providing a starting point for further development and learning.

Reference Links
---------------

1. **S32K148 Evaluation Board Product Page**:
    - `S32K148 Evaluation Board <https://www.nxp.com/design/development-boards/s32k148-evaluation-board:S32K148EVB>`_

2. **S32K148 Evaluation Board Documentation**:
    - `User Manual: S32K148 Evaluation Board User Manual (login required) <https://www.nxp.com/webapp/Download?colCode=S32K148-UM>`_
    - `Quick Start Guide: S32K148 Evaluation Board Quick Start Guide <https://www.nxp.com/document/guide/getting-started-with-the-s32k148-q176-evaluation-board:NGS-S32K148EVB>`_

3. **S32K148 MCU Documentation**:
    - `Reference Manual: S32K148 Reference Manual (login required) <https://www.nxp.com/webapp/Download?colCode=S32K1XXRM>`_
    - `Data Sheet: S32K148 Data Sheet <https://www.nxp.com/docs/en/data-sheet/S32K1xx.pdf>`_

4. **S32K148 Evaluation Board Software and Tools**:
    - `S32 Design Studio for ARM <https://www.nxp.com/design/design-center/software/development-software/s32-design-studio-ide/s32-design-studio-for-arm:S32DS-ARM>`_

5. **Community and Support**:
    - `NXP Community Forum <https://community.nxp.com/>`_
    - `NXP Technical Support <https://www.nxp.com/products/processors-and-microcontrollers/s32-automotive-platform/s32k-auto-general-purpose-mcus/s32k1-microcontrollers-for-automotive-general-purpose:S32K1#support>`_
    - `NXP Community: S32 Design Studio <https://community.nxp.com/t5/S32-Design-Studio/bd-p/s32ds?_gl=1*17wg6i2*_ga*MTM2NTMwNjM3Mi4xNzA0OTc2MjQ1*_ga_WM5LE0KMSH*MTcxMzc3NDMzOC41LjEuMTcxMzc3NTIxNC4wLjAuMA..>`_

Build environment
-----------------
For instructions on building for this platform see :ref:`learning_setup`

Connections
-----------

.. csv-table:: Pin Configuration
   :header: "Name", "State and Function", "Pin ID", "Usage"
   :widths: 10, 20, 15, 20

   "PA0", "IN, GPIO", "EVAL_DI_1", ""
   "PA1", "OUT, GPIO", "EVAL_DO_1", ""
   "PA2", "IN", "EVAL_AI_1", ""
   "PC6", "IN, ALT 2", "UART1_RX", "Serial Debugger Receiver"
   "PC7", "IN, ALT 2", "UART1_TX", ""
   "PC28", "IN", "EVAL_POTI_ADC", "ADC"
   "PE4", "IN, ALT5", "canRx", ""
   "PE5", "OUT, ALT3", "canTx", ""
   "PE21", "OUT, GPIO", "EVAL_LED_RED", "Red LED"
   "PE23", "OUT, ALT2", "EVAL_LED_GREEN", "Green LED"
   "PE22", "OUT, ALT2", "EVAL_LED_BLUE", "Blue LED"
