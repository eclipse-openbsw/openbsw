.. _setup_windows_01_wsl:

Set up Ubuntu :prop:`tool:ubuntu_version` in WSL
================================================

If you have not yet installed WSL (Windows Subsystem for Linux) on your Windows laptop,
both WSL and Ubuntu :prop:`tool:ubuntu_version` can be installed with one command
(this assumes you have Windows 10 version 2004 and higher (Build 19041 and higher) or Windows 11).
But first, you must check that some Windows features are enabled.
From the **Windows Start** menu, find the app named **Turn Windows Features on or off**
and in that check that both these features are enabled...

 * Virtual Machine Platform
 * Windows Subsystem for Linux

If these are not enabled then do so and reboot your laptop.

Assuming the features above are enabled, in a Windows command shell with administrator rights
run the following commands.

Confirm WSL2 as default version...

.. code-block:: bash

    wsl --set-default-version 2

Install Ubuntu :prop:`tool:ubuntu_version`...

.. code-block:: bash

    wsl --install -d Ubuntu-x.x --web-download

During setup of Ubuntu, when prompted to enter a username and password in a Ubuntu terminal,
enter a new username and password of your choice and make a note of it - you may be prompted
for this password in the future when requesting sudo privileges.

If you have issues when trying to set this up on your Windows machine then
`Microsoft's WSL Setup Instructions <https://learn.microsoft.com/en-us/windows/wsl/install>`_
may help.
