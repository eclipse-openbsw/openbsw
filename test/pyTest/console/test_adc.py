# Test applicable for target = s32k148_with_hwtester


def test_adc_ch1(target_session, hw_tester):
    check_mv_val = False
    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()

    Analog_ip_mv = 2700
    hw_tester.write(f"dac write {Analog_ip_mv}\n".encode())
    (stm_write_ok, stm_out, _) = hw_tester.read_until(b"returned 0", timeout=1)
    assert stm_write_ok

    expected = [b"Adc Channel 1 : AiEval_ADC", b"ok"]
    capserial.clear()
    capserial.write(b"adc get 1\n")
    (success, lines, _) = capserial.read_until(expected, timeout=3)
    assert success
    for val in lines:
        if (
            "scaled" in val.decode()
            and abs(int(val.decode().split()[-3]) - Analog_ip_mv) <= 100
        ):
            assert True
