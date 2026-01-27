Setup the test enviroment
  $ . ${TESTDIR}/setup.sh
   RefApp: LIFECYCLE: DEBUG: Run level 9 done\r (esc)

Send a CAN frame to the echo service in the referenceApp
  $ cansend vcan0 "123#AABBCCDDEEFF"

Check that we see the message being sent on the bus
  $ wait_output can 'vcan0  123'
    vcan0  123   [6]  AA BB CC DD EE FF

Check that we see notification about the reception of the message on the serial out
  $ wait_output serial 'received CAN frame, id=0x123, length=6'
   RefApp: CAN: DEBUG: [SocketCanTransceiver] received CAN frame, id=0x123, length=6\r (esc)

Check that we see the echo response on the bus
  $ wait_output can 'vcan0  124'
    vcan0  124   [6]  AA BB CC DD EE FF
