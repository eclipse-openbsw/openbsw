
# `pytest` environment setup

The tests in this directory expect a suitable environment to be in place.
In the instructions below, it is assumed that you have
been able to set up a working build and test environment
following the instructions in the documentation produced from `doc/learning/setup`.
This README only captures those additional steps needed to run `pytest` with the tests in this directory,
which includes setting up the python environment
and setting up each target's requirements as explained below.

## python

Firstly, these tests are written in `python` and executed using `pytest`.
Assuming you have `python` (>=3.10.x) installed,
the additional packages required to run **most** tests can be installed by running...

```
pip install -r requirements.txt
```

Additionally, tests that use `udstool` (such as `uds/test_udsToolRDBI.py`)
need it installed as follows...

```
cd ../../tools/UdsTool
pip install .
```

## `pytest` options

`pytest`'s [basic command-line usage page](https://docs.pytest.org/en/stable/how-to/usage.html)
introduces the many options it provides.
In addition to these built-in options,
if you run `pytest -h` in this directory you will see these custom options...
```
Custom options:
  --target=TARGET       Name of target where the file "target_[TARGET].toml" exists.
                        eg.1: "--target=posix" reads from "target_posix.toml".
                        Can be specified multiple times.
                        Tests with the fixture "target_session" are parameterized
                        to run once for each target specified.
                        eg.2: "--target=posix --target=s32k148"
                        reads from "target_posix.toml" and "target_s32k148.toml" and
                        each test with fixture "target_session" runs for both targets.
                        Supports "*".
                        eg.3: "--target=*" reads from files matching "target_*.toml".
                        Default="posix".
  --no-restart          Skip restart of target(s) before each test
```
The `--target` option allows you to choose the target to run tests with.
Each target's TOML file defines its details (see format description below).

By default, each target is started before running each test and stopped at the end of each test,
but you can override that using the `--no-restart` option so that it will
launch the target once at the start of the `pytest` run and stop it at the end.

### Running `pytest` with `--target=posix`

The file `target_posix.toml` defines how tests will run for the `posix` target.
If you examine that and the scripts it references you will find that it expects the following...

1. Need to have built `posix`

    To run tests using the `posix` you must first have built the image for that target
    in its expected default location...
    ```
    cmake-build-posix/application/app.referenceApp.elf
    ```

2. Need to have `vcan0` set up

    The `posix` build has `vcan0` hardcoded
    as the name of the `SocketCAN` interface in use,
    so this must be set up for tests involving `CAN` to pass.

3. Need to have tap0 set up

    The posix build uses a ethernet interface named tap0.
    Run ./tools/enet/bring-up-ethernet.sh before pytest to set up tap0.

### Running `pytest` with `--target=s32k148`

For this option...

1. Need to have built an image for the `s32k148` target

    To run tests using the `s32k148` you must first have built the image for that target
    in its expected default location...
    ```
    cmake-build-s32k148/application/app.referenceApp.elf
    ```

2. Need an [S32K148 development board](https://www.nxp.com/design/design-center/development-boards/automotive-development-platforms/s32k-mcu-platforms/s32k148-q176-evaluation-board-for-automotive-general-purpose:S32K148EVB)

    You need to have the board running, connected via USB to your build machine.

2. Need to have `can0` set up

    `target_s32k148.toml` in this directory specifies `can0`
    as the name of the `SocketCAN` interface to use
    to communicate with the S32K148 development board,
    so this is expected be set up for tests involving `CAN` to pass.

3. Need `pegdbserver_console` up and running

    You need P&E Micro's version of `gdbserver` running and connected to the board...
    ```
    sudo ./pegdbserver_console -startserver -device=NXP_S32K1xx_S32K148F2M0M11
    ```
    This allows the `.gdb` scripts referenced by `target_s32k148.toml` to flash
    the board at the start of `pytest` and to reset the board before each test.

### Running `pytest` with `--target=s32k148_with_hwtester`

For this option, you need everything set up as for `--target=s32k148` **and**
the `s32k148` board must be set up with an external hardware testing board
which has been configured for serial communication in `target_s32k148_with_hwtester.toml`
under `[hw_tester_serial]`.
Tests with the fixture `hw_tester` will be only be run if `[hw_tester_serial]` is present.

## Examples

To run all tests for `posix` run...
```
pytest
```
To run all tests for `s32k148` without restarting between tests, run...
```
pytest --target=s32k148 --no-restart
```
To run all tests found in one file on all targets (for which `target_*.toml` files are found)...
```
pytest --target=* can/test_can.py
```
To run all test files in the `uds` subdirectory, but ignore one, for `s32k148`...
```
pytest --target=s32k148 --ignore=uds/test_udsToolRDBI.py uds/
```

## Format of `target_*.toml` files

The `target_*.toml` files in this directory contain configuration for each target.
See those for full examples.

Note that the same command-line options that are added to `pytest`
are implemented in `target_info.py` such that you can use this file directly.
Eg.1 `python target_info.py -h` prints the same help that appears in
the output from `pytest -h` under `Custom options`.
Eg.2 `python target_info.py --target=* --no-restart` will read all target
files, apply the option `--no-restart` and print the resultant setup values.
This allows you to edit .toml files and check they can be successfully read
before using them in `pytest`.

The following headers and the values beneath each are supported.

### `[socketcan]`
Entries under this heading are passed directly to the `SocketcanBus` class constructor and so must match an argument it supports. See
python-can's [SocketcanBus class](https://python-can.readthedocs.io/en/stable/interfaces/socketcan.html#can.interfaces.socketcan.SocketcanBus).

eg.
```
[socketcan]
channel = "can0"
```

### `[serial]`
Entries under this heading are passed to the `Serial` class constructor
(from which `CaptureSerial` derives). See
pyserial's [Serial class](https://pyserial.readthedocs.io/en/latest/pyserial_api.html#serial.Serial)
In addition to `Serial` class constructor arguments, the following is also supported...
| Name         | Mandatory   | Type      | Value     |
| ------------ | ----------- | --------- | --------- |
| `write_byte_delay` |       | Float     | The time in seconds to delay before writing each byte |
| `send_command_expected`    |           | String    | The string to look for in serial output that indicates `send_command` was successful. |
| `send_command_timeout`     |           | Float     | The time (in seconds) for `send_command` to wait for `expected`. |
| `send_command_max_retries` |           | Integer   | The number of times for `send_command` to retry sending a command and waiting for `expected`. (Default = `0`) |
eg.
```
[serial]
port = "/dev/ttyACM0"
baudrate = 115200
write_byte_delay = 0.01
send_command_expected = "Console command succeeded"
send_command_timeout = 0.1
send_command_max_retries = 2
```
Note that...

* There is one special value supported for `port`.
  If it is set to `"PTY_FORWARDER"` then this value will be replaced
  with the path to a pseudo terminal created internally.
  If this is set then the `[pty_forwarder]` section is required.
* The method `CaptureSerial.send_command()` will only work if `send_command_expected` and `send_command_timeout` are defined
  whereas `send_command_max_retries` is optional.

### `[eth]`
Entries under this heading are used to configure ethernet ip.

eg.
```
[eth]
ip_address = "192.168.0.200"
```

### [pty_forwarder]
If `port = "PTY_FORWARDER"` in the `[serial]` section then this section is required and must have the entry `ext_link`
which will be a link created to a pseudo terminal created internally, but usable externally.
If the input and output of the target's console is redirected to this link
then tests can interact with the console. See the example in `[target_process]`.

| Name        | Mandatory | Type      | Value     |
| ----------- | --------- | --------- | --------- |
| `ext_link`  | Yes       | String | The path a link that will be created to which I/O to/from the console can be redirected |
eg.
```
[pty_forwarder]
ext_link = "/tmp/pty_forwarder"
```

### `[per_run_process]`

If omitted, then it is assumed nothing is required to be run externally
at the start of testing.
If present, the following entries are supported.

| Name            | Mandatory | Type      | Value     |
| --------------- | --------- | --------- | --------- |
| `command_line`  | Yes       | String | The command to run before all tests |
| `wait_for_exit` | | Boolean | If `true` then wait until the command exits before proceeding with tests. Note that while the process of the command may end, other child processes it launched may still be running. This option only waits for the command's process to exit. If `false` (the default) do not wait, just kick off the command. |
| `restart_if_exited` | | Boolean | If `true` then monitor the process and restart it if it stops. If `false` (the default) do nothing. This option only applies when `wait_for_exit` is `false` |
| `kill_at_end`  | | Boolean | If `true` then when all testing is complete, send `SIGTERM` to all child processes started by the command. If `false` (the default) do nothing. |

eg.
```
[per_run_process]
command_line = "arm-none-eabi-gdb -batch -x flash.gdb ../../cmake-build-s32k148/application/app.referenceApp.elf > /dev/null 2>&1"
wait_for_exit = true
```

### `[target_process]`

If present, it is assumed that the command executed will result in
restarting the target.
The following entries are supported.

| Name            | Mandatory | Type      | Value     |
| --------------- | --------- | --------- | --------- |
| `command_line`  | Yes       | String | The command to run before each test. |
| `wait_for_exit` | | Boolean | If `true` then wait until the command exits before proceeding with each test. Note that while the process of the command may end, other child processes it launched may still be running. This option only waits for the command's process to exit. If `false` (the default) do not wait, just kick off the command. |
| `restart_if_exited` | | Boolean | If `true` then monitor the process and restart it if it stops. If `false` (the default) do nothing. This option only applies when `wait_for_exit` is `false` |
| `kill_at_end`  | | Boolean | If `true` then when each test is complete, send `SIGTERM` to all child processes started by the command. If `false` (the default) do nothing. |
| `skip_first`  | | Boolean | If `true` then only run the command before the 2nd and subsequent tests, not before the first test. `false` is the default. |

eg.
```
[target_process]
command_line = "../../cmake-build-posix/application/app.referenceApp.elf < /tmp/pty_forwarder > /tmp/pty_forwarder"
kill_at_end = true
```

If the `[target_process]` section is omitted, then the target is assumed to be available
and reachable with the settings in `[serial]` and `[socketcan]`,
the target will not be restarted between tests
and the function `target_session.restart()` will not work.
But it is possible to run some tests without this section if `assume_booted_at_start`
is used in the `[boot]` section.

### `[boot]`


| Name           | Mandatory | Type      | Value     |
| -------------- | --------- | --------- | --------- |
| `started_str`  | Yes       | String | The string to look for in serial output that marks the beginning of boot. |
| `complete_str` | Yes       | String | The string to look for in serial output that marks the end of boot. |
| `wait_time_after_complete` | | Float | If set (in seconds) then the target will be marked as booted after this time has elapsed after `complete_str` is received. |
| `max_time`     | Yes       | Float  | The time (in seconds) to wait for boot to complete. |
| `assume_booted_at_start` | | Boolean | If `true`, assume the target is booted when `pytest` starts. If `false` (the default), assume the target is not booted when `pytest` starts. This only has an effect if `[target_process]` is omitted. |

eg.
```
[boot]
started_str = "INFO: Run level 1"
complete_str = "DEBUG: Run level 8 done"
max_time = 0.5
```

### `[hw_tester_serial]`
If present, it describes the serial parameters to communicate with
an external board that is connected to the target board for hardware testing.
Tests with the fixture `hw_tester` will only run for targets with `[hw_tester_serial]` defined,
and in those tests, `hw_tester` is a `CaptureSerial` object.
This section accepts the same parameters as for `[serial]` above,
except that `"PTY_FORWARDER"` is not a supported value for `port`.

eg.
```
[hw_tester_serial]
port = "/dev/ttyACM1"
baudrate = 115200
write_byte_delay = 0.01
send_command_expected = "returned 0"
send_command_timeout = 0.1
send_command_max_retries = 2
```

## Unit tests

Some unit tests of utility classes are in the `unittests` subdirectory.
These are not part of and are excluded from `pytest`
(as `.pytest.ini` contains `norecursedirs = unittests`).
These should be run from this directory, like this...
```
python -m unittest unittests/*.py
```
