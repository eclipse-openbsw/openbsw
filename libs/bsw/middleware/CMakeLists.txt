add_library(middlewareInterfaces INTERFACE)
target_include_directories(middlewareInterfaces INTERFACE interfaces/include)
target_link_libraries(middlewareInterfaces INTERFACE etl)

# Shared base of middlewareSig and middlewareSvc: memory pools and queues.
add_library(middleware src/middleware/memory/PoolBase.cpp)

target_include_directories(middleware PUBLIC include)
target_include_directories(middleware PRIVATE interfaces/include)
target_link_libraries(middleware PUBLIC etl middlewareInterfaces)

# Signal-based communication.
add_library(
    middlewareSig
    src/middleware/sig/core/MiddlewarePduBase.cpp
    src/middleware/sig/core/PduBrokerConnectionBase.cpp
    src/middleware/sig/core/PhysicalSignalConversion.cpp)

target_link_libraries(middlewareSig PUBLIC middleware)

# Service-based communication.
add_library(
    middlewareSvc
    src/middleware/svc/core/ClusterConnection.cpp
    src/middleware/svc/core/ClusterConnectionBase.cpp
    src/middleware/svc/core/DatabaseManipulator.cpp
    src/middleware/svc/core/EventSender.cpp
    src/middleware/svc/core/IClusterConnectionConfigurationBase.cpp
    src/middleware/svc/core/LoggerApi.cpp
    src/middleware/svc/core/MessagePayloadBuilder.cpp
    src/middleware/svc/core/ProxyBase.cpp
    src/middleware/svc/core/ResponseBufferBase.cpp
    src/middleware/svc/core/SkeletonBase.cpp
    src/middleware/svc/rpc/ProxyFireAndForgetMethod.cpp
    src/middleware/svc/rpc/ProxyMethodImpl.cpp)

target_link_libraries(middlewareSvc PUBLIC middleware)
