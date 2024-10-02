add_library(socketCanTransceiver src/can/SocketCanTransceiver.cpp)

target_include_directories(socketCanTransceiver PUBLIC include)

target_link_libraries(socketCanTransceiver PUBLIC cpp2can io)
