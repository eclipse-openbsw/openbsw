add_library(
    routing
    src/routing/pduRouting.cpp
    src/routing/RxAdapterTable.cpp
    src/routing/TxAdapterTable.cpp
    src/routing/Header.cpp
    src/routing/LegacyTxAdapter.cpp
    src/routing/util.cpp
    src/routing/Logger.cpp
    src/routing/PduTransportTxAdapter.cpp
    src/routing/ChannelNames.cpp
    src/routing/PduTransportBufferedWriter.cpp
    src/routing/RxAdapter.cpp
    src/routing/PduTransportConfig.cpp)

add_library(routingConfiguration INTERFACE)

target_include_directories(routing PUBLIC include)

target_link_libraries(
    routing
    PUBLIC blob
           bsp
           cpp2ethernet
           etl
           io
           lwipSocket
           platform
           routingConfiguration
           util)
