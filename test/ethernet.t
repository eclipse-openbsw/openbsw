Setup the test enviroment
  $ . ${TESTDIR}/setup.sh
   RefApp: LIFECYCLE: DEBUG: Run level 9 done\r (esc)

Just send two pings and see that we do get a response
  $ ping -c 2 -W 2 $TARGET_IP
  PING *.*.*.* (*.*.*.*) 56(84) bytes of data. (glob)
  64 bytes from *.*.*.*: icmp_seq=1 ttl=255 time=* ms (glob)
  64 bytes from *.*.*.*: icmp_seq=2 ttl=255 time=* ms (glob)
  
  --- *.*.*.* ping statistics --- (glob)
  2 packets transmitted, 2 received, 0% packet loss, time * (glob)
  rtt min/avg/max/mdev = */*/*/* ms (glob)
