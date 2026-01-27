Setup the test enviroment
  $ . ${TESTDIR}/setup.sh
   RefApp: LIFECYCLE: DEBUG: Run level 9 done\r (esc)

  $ advance_output serial
  $ send_serial "logger level"

Use the "logger level" command to check that it reports all components initially at "DEBUG" level
  $ wait_output serial 'Received console command "logger level"'
   RefApp: CONSOLE: INFO: Received console command "logger level"\r (esc)
  $ wait_output serial 'BSP             DEBUG'
   BSP             DEBUG\r (esc)
  $ wait_output serial 'COMMON          DEBUG'
   COMMON          DEBUG\r (esc)
  $ wait_output serial 'DEMO            DEBUG'
   DEMO            DEBUG\r (esc)
  $ wait_output serial 'GLOBAL          DEBUG'
   GLOBAL          DEBUG\r (esc)
  $ wait_output serial 'LIFECYCLE       DEBUG'
   LIFECYCLE       DEBUG\r (esc)
  $ wait_output serial 'CONSOLE         DEBUG'
   CONSOLE         DEBUG\r (esc)
  $ wait_output serial 'CAN             DEBUG'
   CAN             DEBUG\r (esc)
  $ wait_output serial 'DOCAN           DEBUG'
   DOCAN           DEBUG\r (esc)
  $ wait_output serial 'UDS             DEBUG'
   UDS             DEBUG\r (esc)
  $ wait_output serial 'TPROUTER        DEBUG'
   TPROUTER        DEBUG\r (esc)
  $ wait_output serial 'ok'
   ok\r (esc)
  $ wait_output serial 'Console command succeeded'
   RefApp: CONSOLE: INFO: Console command succeeded\r (esc)

Set some components to different log levels
  $ advance_output serial
  $ send_serial "logger level BSP error"
  $ wait_output serial 'Console command succeeded'
   RefApp: CONSOLE: INFO: Console command succeeded\r (esc)

  $ advance_output serial
  $ send_serial "logger level COMMON info"
  $ wait_output serial 'Console command succeeded'
   RefApp: CONSOLE: INFO: Console command succeeded\r (esc)

  $ advance_output serial
  $ send_serial "logger level DEMO critical"
  $ wait_output serial 'Console command succeeded'
   RefApp: CONSOLE: INFO: Console command succeeded\r (esc)

Check the level changes are reported
  $ advance_output serial
  $ send_serial "logger level"
  $ wait_output serial 'BSP             ERROR'
   BSP             ERROR\r (esc)
  $ wait_output serial 'COMMON          INFO'
   COMMON          INFO\r (esc)
  $ wait_output serial 'DEMO            CRITICAL'
   DEMO            CRITICAL\r (esc)
  $ wait_output serial 'GLOBAL          DEBUG'
   GLOBAL          DEBUG\r (esc)
  $ wait_output serial 'LIFECYCLE       DEBUG'
   LIFECYCLE       DEBUG\r (esc)
  $ wait_output serial 'CONSOLE         DEBUG'
   CONSOLE         DEBUG\r (esc)
  $ wait_output serial 'CAN             DEBUG'
   CAN             DEBUG\r (esc)
  $ wait_output serial 'DOCAN           DEBUG'
   DOCAN           DEBUG\r (esc)
  $ wait_output serial 'UDS             DEBUG'
   UDS             DEBUG\r (esc)
  $ wait_output serial 'TPROUTER        DEBUG'
   TPROUTER        DEBUG\r (esc)
  $ wait_output serial 'Console command succeeded'
   RefApp: CONSOLE: INFO: Console command succeeded\r (esc)

The CAN logger should only log received frames when in DEBUG.
Verify this by sending a frame both while in DEBUG and while in another
level.
  $ advance_output serial
  $ advance_output can
  $ cansend vcan0 "123#AABBCCDDEEFF"
  $ wait_output can 'vcan0  123'
    vcan0  123   [6]  AA BB CC DD EE FF
  $ wait_output serial 'received CAN frame, id=0x123, length=6'
   RefApp: CAN: DEBUG: [SocketCanTransceiver] received CAN frame, id=0x123, length=6\r (esc)
  $ advance_output serial
  $ advance_output can

  $ send_serial "logger level CAN info"
  $ wait_output serial 'Console command succeeded'
   RefApp: CONSOLE: INFO: Console command succeeded\r (esc)
  $ advance_output serial
  $ advance_output can
  $ cansend vcan0 "123#AABBCCDDEEFF"
  $ wait_output can 'vcan0  123'
    vcan0  123   [6]  AA BB CC DD EE FF
  $ wait_output serial 'received CAN frame, id=0x123, length=6'
  [1]
  $ advance_output serial
  $ advance_output can
