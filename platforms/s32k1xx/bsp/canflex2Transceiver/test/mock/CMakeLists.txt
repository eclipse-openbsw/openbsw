add_library(canflex2TransceiverMock INTERFACE)

target_include_directories(canflex2TransceiverMock INTERFACE include)

target_link_libraries(canflex2TransceiverMock INTERFACE canflex2Transceiver
                                                        cpp2can gmock)
