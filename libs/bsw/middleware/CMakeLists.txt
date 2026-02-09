add_library(middlewareHeaders INTERFACE)

target_include_directories(
    middlewareHeaders
    INTERFACE $<INSTALL_INTERFACE:include>
              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_link_libraries(middlewareHeaders INTERFACE etl)

add_library(
    middleware
    src/middleware/core/ClusterConnection.cpp
    src/middleware/core/ClusterConnectionBase.cpp
    src/middleware/core/DatabaseManipulator.cpp
    src/middleware/core/IClusterConnectionConfigurationBase.cpp
    src/middleware/core/LoggerApi.cpp
    src/middleware/core/ProxyBase.cpp
    src/middleware/core/SkeletonBase.cpp)

target_include_directories(middleware PUBLIC include)

target_link_libraries(middleware PUBLIC etl)
