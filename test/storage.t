Setup the test enviroment
  $ . ${TESTDIR}/setup.sh
   RefApp: LIFECYCLE: DEBUG: Run level 9 done\r (esc)

This tests non-volatile storage read/write via console command.
We write a value, reboot the target and read it back to check it survived
a lifecycle reboot.
This is repeated with two different data values at the same address to make
sure we are not passing this test with a leftover written value from a previous
test run.

  $ advance_output serial
  $ send_serial "storage write 2561 aabbccdd"
  $ wait_output serial 'Console command succeeded'
   RefApp: CONSOLE: INFO: Console command succeeded\r (esc)
  $ send_serial "storage read 2561 4"
  $ wait_output serial "aabbccdd  "
   aabbccdd        \r (no-eol) (esc)
  \r (esc)

  $ send_serial "lc reboot"
  $ advance_output serial
  $ wait_output serial 'Run level 8 done'
   RefApp: LIFECYCLE: DEBUG: Run level 8 done\r (esc)
  $ send_serial "storage read 2561 4"
  $ wait_output serial "aabbccdd  "
   aabbccdd        \r (no-eol) (esc)
  \r (esc)


  $ advance_output serial
  $ send_serial "storage write 2561 ddccbbaa"
  $ wait_output serial 'Console command succeeded'
   RefApp: CONSOLE: INFO: Console command succeeded\r (esc)
  $ send_serial "storage read 2561 4"
  $ wait_output serial "ddccbbaa  "
   ddccbbaa        \r (no-eol) (esc)
  \r (esc)

  $ send_serial "lc reboot"
  $ advance_output serial
  $ wait_output serial 'Run level 8 done'
   RefApp: LIFECYCLE: DEBUG: Run level 8 done\r (esc)
  $ send_serial "storage read 2561 4"
  $ wait_output serial "ddccbbaa  "
   ddccbbaa        \r (no-eol) (esc)
  \r (esc)
