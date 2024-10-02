add_library(udsMock gmock/src/uds/session/DiagSessionMock.cpp)

target_include_directories(udsMock PUBLIC gmock/include)

target_link_libraries(
    udsMock
    PUBLIC gmock udsConfiguration
    PRIVATE uds)
