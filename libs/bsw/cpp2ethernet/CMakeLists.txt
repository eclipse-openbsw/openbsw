add_library(
    cpp2ethernet
    src/ethernet/EthernetLogger.cpp
    src/tcp/socket/AbstractServerSocket.cpp
    src/tcp/socket/AbstractSocket.cpp
    src/tcp/TcpLogger.cpp
    src/tcp/util/LoopbackTestServer.cpp
    src/tcp/util/BandwidthTestSocket.cpp
    src/tcp/util/TcpIperf2Server.cpp
    src/udp/socket/AbstractDatagramSocket.cpp
    src/udp/UdpLogger.cpp
    src/udp/DatagramPacket.cpp
    src/udp/util/UdpEchoTestServer.cpp
    src/udp/util/UdpIperf2Server.cpp
    src/ip/NetworkInterfaceConfig.cpp
    src/ip/to_str.cpp)

target_include_directories(cpp2ethernet PUBLIC include)

target_link_libraries(
    cpp2ethernet
    PUBLIC common
           bsp
           etl
           util
           async)
