add_library(tapEthernetDriver src/TapEthernetDriver.cpp)

target_include_directories(tapEthernetDriver PUBLIC include)

target_link_libraries(tapEthernetDriver PUBLIC async lwipSocket)
