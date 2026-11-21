add_library(
    middlewareSomeIp
    src/middleware/bindings/someip/ConsumerEndpoint.cpp
    src/middleware/bindings/someip/MemberTable.cpp
    src/middleware/bindings/someip/PayloadTransfer.cpp
    src/middleware/bindings/someip/ProviderEndpoint.cpp)

target_include_directories(middlewareSomeIp PUBLIC include)

target_link_libraries(middlewareSomeIp PUBLIC cpp2someip middleware)
