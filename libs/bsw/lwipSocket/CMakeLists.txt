add_library(
    lwipSocket
    src/lwipSocket/tcp/LwipSocket.cpp
    src/lwipSocket/tcp/LwipServerSocket.cpp
    src/lwipSocket/netif/LwipNetworkInterface.cpp
    src/lwipSocket/udp/LwipDatagramSocket.cpp
    src/lwipSocket/utils/TaskAssert.cpp
    src/lwipSocket/utils/FilterFrame.cpp)

target_include_directories(lwipSocket PUBLIC include)

target_link_libraries(
    lwipSocket
    PUBLIC common
           etl
           util
           async
           lwipcore
           cpp2ethernet
    PRIVATE)
