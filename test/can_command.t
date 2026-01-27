Setup the test enviroment
  $ . ${TESTDIR}/setup.sh
   RefApp: LIFECYCLE: DEBUG: Run level 9 done\r (esc)

Send a can frame via console command
  $ send_serial "can send 0x100 100 2 3 46 7 20"

Check that we see the frame on the bus
  $ wait_output can 'vcan0  100'
    vcan0  100   [6]  64 02 03 2E 07 14

Same for hex bytes
  $ send_serial "can send 0x101 0xA1 0xB2 0xC3 0xD4 0xE5 0xF6 0xA7 0xB8"
  $ wait_output can 'vcan0  101'
    vcan0  101   [8]  A1 B2 C3 D4 E5 F6 A7 B8
