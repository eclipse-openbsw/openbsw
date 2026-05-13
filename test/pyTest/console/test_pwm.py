# Integration test: set PWM duty via console and validate output using RC filter + ADC
# PWM Channel: FTM4_CH4(PTE24) : J5-7
# ADC Channel: ADC0(PTC29) : J4-10
# RC filter: R=10k, C=0.1uF

import pytest
import time


@pytest.mark.skip_if("target!=['s32k148']")
@pytest.mark.parametrize("duty_cycle", [0, 2500, 5000, 7500, 10000])
def test_pwm_rc_filter(target_session, duty_cycle):
    if target_session.target_name == "posix":
        pytest.skip("Hardware specific test")

    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()

    write_command = f"pwm set 3 {duty_cycle}\n"
    write_expected = [
        b"ok",
        f'Received console command "pwm set 3 {duty_cycle}"'.encode(),
        b"Console command succeeded",
    ]
    capserial.clear()
    capserial.send_command(write_command.encode())
    success, output, _ = capserial.read_until(
        write_expected, timeout=2, match_all_expected=True
    )
    assert success, f"pwm set failed for {duty_cycle}"

    # give RC filter + output driver time to settle
    time.sleep(0.05)

    # Convert duty (0..10000) to expected voltage in mV
    VDD = 5
    expected_voltage_mv = int(VDD * (duty_cycle / 10000.0) * 1000.0)
    tolerance_mv = 100
    expected = [b"Adc Channel 1 : AiEval_ADC", b"ok"]
    capserial.clear()
    capserial.write(b"adc get 1\n")
    (success, lines, _) = capserial.read_until(expected, timeout=3)
    assert success, "Failed to read ADC channel 1"

    # Validate ADC measurement
    valid_measurement = False
    for val in lines:
        if "scaled" in val.decode():
            measured_mv = int(val.decode().split()[-3])
            if abs(measured_mv - expected_voltage_mv) <= tolerance_mv:
                valid_measurement = True
            else:
                raise AssertionError(
                    f"ADC value {measured_mv} mV is out of range (±{tolerance_mv} mV of {expected_voltage_mv} mV)"
                )
    assert valid_measurement, "No valid ADC measurement found within expected range"
