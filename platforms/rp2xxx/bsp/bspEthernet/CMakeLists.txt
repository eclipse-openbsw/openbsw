add_library(bspEthernet src/ethernet/EnetDriver.cpp src/ethernet/RxBuffers.cpp
                        src/ethernet/TxBuffers.cpp)

target_include_directories(bspEthernet PUBLIC include)

target_link_libraries(
    bspEthernet
    bsp
    bspInterrupts
    bspMcu
    lwipcore
    util
    cpp2ethernet
    lwipSocket
    bspTja1101)
