Setup the test enviroment
  $ . ${TESTDIR}/setup.sh
   RefApp: LIFECYCLE: DEBUG: Run level 9 done\r (esc)

  $ cansend vcan0 "02A#023E00000000000000"

  $ wait_output can 'vcan0  02A'
    vcan0  02A   [8]  02 3E 00 00 00 00 00 00
  $ wait_output can 'vcan0  0F0'
    vcan0  0F0   [8]  02 7E 00 CC CC CC CC CC
