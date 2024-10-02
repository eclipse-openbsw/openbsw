add_library(
    transportRouterSimple src/transport/routing/TransportRouterSimple.cpp
                          src/transport/TpRouterLogger.cpp)

target_include_directories(transportRouterSimple PUBLIC include)

target_link_libraries(
    transportRouterSimple
    PUBLIC transport transportConfiguration
    PRIVATE configuration)
