Setup the test enviroment
  $ . ${TESTDIR}/setup.sh
   RefApp: LIFECYCLE: DEBUG: Run level 9 done\r (esc)

Advance so we don't match previous output
  $ advance_output serial

Send reboot command
  $ send_serial "lc reboot"

Check that we have reached shutdown
  $ wait_output serial 'Lifecycle shutdown complete'
   RefApp: LIFECYCLE: INFO: Lifecycle shutdown complete\r (esc)

And also check that we booted again
  $ wait_output serial 'Run level 8 done'
   RefApp: LIFECYCLE: DEBUG: Run level 8 done\r (esc)
