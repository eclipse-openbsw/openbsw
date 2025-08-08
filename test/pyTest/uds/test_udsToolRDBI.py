import os
import helper
from target_info import TargetInfo


def create_uds_tool_command(channel, did):
    config = os.path.join(os.getcwd(), "../../tools/UdsTool/app/canConfig.json")
    return f"udstool read --can --channel {channel} --txid 0x2A --rxid 0xF0 --did {did} --config {config}"


def test_rdbi(target_session, did="CF01"):
    assert target_session.capserial().wait_for_boot_complete()

    command = create_uds_tool_command(
        target_session.target_info.socketcan["channel"], did
    )
    output = helper.run_process(command)

    assert any(
        "PositiveResponse" in line for line in output
    ), "Missing PositiveResponse"
    assert any(
        "62cf01010200022202160f0100006d2f0000010600008fe0000001" in line
        for line in output
    ), "Incorrect expected output"
