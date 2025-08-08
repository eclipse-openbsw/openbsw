from glob import glob
import tomli
from pathlib import Path

# Note that the same command-line options that are added to pytest
# are implemented below such that you can use this file directly.
# Eg.1 "python target_info.py -h" prints the same help that appears in
# the output from "pytest -h" under "Custom options"
# Eg.2 "python target_info.py --target=* --no-restart" will read all target
# files, apply the option --no-restart and print the resultant setup values.
# This allows you to edit .toml files and check they can be successfully read
# before using them in pytest


class TargetInfo():
    """Reads target configuration from a .toml file
    and makes it available through read-only properties.
    """

    by_name = {}

    target_arg_help = (
        "Name of target where the file \"target_[TARGET].toml\" exists. \n"
        "eg.1: \"--target=posix\" reads from \"target_posix.toml\". \n"
        "Can be specified multiple times. \n"
        "Tests with the fixture \"target_session\" are parameterized \n"
        "to run once for each target specified. \n"
        "eg.2: \"--target=posix --target=s32k148\" \n"
        "reads from \"target_posix.toml\" and \"target_s32k148.toml\" and\n"
        "each test with fixture \"target_session\" runs for both targets. \n"
        "Supports \"*\". \n"
        "eg.3: \"--target=*\" reads from files matching \"target_*.toml\". \n"
        "Default=\"posix\".")

    def __init__(self, name, fp, cmd_line_no_restart):
        self._name = name
        target_info = tomli.load(fp)
        self._socketcan = target_info.get("socketcan", {})
        self._serial = target_info.get("serial", {})
        self._eth = target_info.get("eth", {})
        self._pty_forwarder = target_info.get("pty_forwarder", {})
        self._per_run_process = target_info.get("per_run_process", {})
        self._target_process = target_info.get("target_process", {})
        self._boot = target_info.get("boot", {})
        self._hw_tester_serial = target_info.get("hw_tester_serial", {})
        self._set_defaults(cmd_line_no_restart)

    def _set_defaults(self, cmd_line_no_restart):
        if cmd_line_no_restart:
            self._target_process["restart_before_test"] = False
        else:
            if "restart_before_test" not in self._target_process:
                self._target_process["restart_before_test"] = True

    @property
    def name(self):
        """The target name.
        Taken from [TARGET] in the file name \"target_[TARGET].toml\"
        """
        return self._name[:]

    @property
    def socketcan(self):
        """A copy of the dictionary read from under the "socketcan" heading
        in the target's TOML file.
        Note that this is passed as the arguments to
        can.interfaces.socketcan.SocketcanBus() from the python-can library.
        """
        return self._socketcan.copy()

    @property
    def serial(self):
        """A copy of the dictionary read from under the "serial" heading
        in the target's TOML file.
        Note that this is passed as the arguments to
        serial.Serial() from the pyserial library
        from which CaptureSerial is derived.
        """
        return self._serial.copy()

    @property
    def eth(self):
        """A copy of the dictionary read from under the "ip" heading
        in the target's TOML file.
        """
        return self._eth.copy()

    @property
    def pty_forwarder(self):
        """A copy of the dictionary read from under the "pty_forwarder" heading
        in the target's TOML file.
        """
        return self._pty_forwarder.copy()

    @property
    def per_run_process(self):
        """A copy of the dictionary read from under the "per_run_process"
        heading in the target's TOML file.
        """
        return self._per_run_process.copy()

    @property
    def target_process(self):
        """A copy of the dictionary read from under the "target_process"
        heading in the target's TOML file, with default value added
        if needed for "restart_before_test".
        """
        return self._target_process.copy()

    @property
    def boot(self):
        """A copy of the dictionary read from under the "boot"
        heading in the target's TOML file.
        """
        return self._boot.copy()

    @property
    def hw_tester_serial(self):
        """A copy of the dictionary read from under the "hw_tester_serial"
        heading in the target's TOML file.
        """
        return self._hw_tester_serial.copy()

    def __str__(self):
        desc = f"name={self.name}\n"
        desc = desc + f"socketcan={self.socketcan}\n"
        desc = desc + f"serial={self.serial}\n"
        desc = desc + f"ip_addr={self.ip_addr}\n"
        desc = desc + f"pty_forwarder={self.pty_forwarder}\n"
        desc = desc + f"per_run_process={self.per_run_process}\n"
        desc = desc + f"target_process={self.target_process}\n"
        desc = desc + f"boot={self.boot}\n"
        desc = desc + f"hw_tester_serial={self.hw_tester_serial}\n"
        return desc

    @staticmethod
    def load(targets, cmd_line_no_restart):
        if not targets:
            targets = ["posix"]
        elif isinstance(targets, str):
            targets = [targets]
        for target in targets:
            if not target.startswith("target_"):
                target = "target_"+target
            if not target.endswith(".toml"):
                target = target+".toml"
            for file_name in glob(target):
                with open(file_name, mode="rb") as fp:
                    name = Path(file_name).stem[len("target_"):]
                    TargetInfo.by_name[name] = TargetInfo(name,
                                                          fp,
                                                          cmd_line_no_restart)


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(
        description='Test loading target files.',
        formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument(
        "--target",
        action="append",
        default=[],
        help=TargetInfo.target_arg_help
    )
    parser.add_argument(
        "--no-restart",
        action="store_true",
        help="Skip restart of target(s) before each test",
    )
    args = parser.parse_args()

    TargetInfo.load(args.target, args.no_restart)

    for name in TargetInfo.by_name:
        print("target \""+name+"\" read from target_"+name+".toml")
        print(TargetInfo.by_name[name])
