add_library(middlewareInterfaces INTERFACE)
target_include_directories(middlewareInterfaces INTERFACE interfaces/include)
target_link_libraries(middlewareInterfaces INTERFACE etl)

add_library(
    middleware
    src/middleware/core/ClusterConnection.cpp
    src/middleware/core/ClusterConnectionBase.cpp
    src/middleware/core/DatabaseManipulator.cpp
    src/middleware/core/EventSender.cpp
    src/middleware/core/FutureDispatcherBase.cpp
    src/middleware/core/IClusterConnectionConfigurationBase.cpp
    src/middleware/core/LoggerApi.cpp
    src/middleware/core/MessagePayloadBuilder.cpp
    src/middleware/core/ProxyBase.cpp
    src/middleware/core/ResponseBufferBase.cpp
    src/middleware/core/SkeletonBase.cpp
    src/middleware/memory/PoolBase.cpp)

target_include_directories(middleware PUBLIC include)
target_include_directories(middleware PRIVATE interfaces/include)
target_link_libraries(middleware PUBLIC etl middlewareInterfaces)
