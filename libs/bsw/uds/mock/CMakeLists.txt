add_library(udsMock gmock/src/uds/session/DiagSessionMock.cpp)

target_include_directories(
    udsMock
    PUBLIC gmock/include
    PRIVATE $<TARGET_PROPERTY:uds,INTERFACE_INCLUDE_DIRECTORIES>)

target_link_libraries(udsMock PUBLIC gmock udsConfiguration)