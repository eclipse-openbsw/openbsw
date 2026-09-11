add_library(
    doip
    src/doip/common/DoIpCommonLogger.cpp
    src/doip/common/DoIpCyclicTaskGenerator.cpp
    src/doip/common/DoIpSendJobHelper.cpp
    src/doip/common/DoIpStaticPayloadSendJob.cpp
    src/doip/common/DoIpTcpConnection.cpp
    src/doip/common/DoIpTransportMessageProvidingListenerAdapter.cpp
    src/doip/common/DoIpTransportMessageProvidingListenerHelper.cpp
    src/doip/common/DoIpTransportMessageSendJob.cpp
    src/doip/common/DoIpUdpConnection.cpp
    src/doip/common/DoIpVehicleIdentificationRequestSendJob.cpp
    src/doip/server/DoIpServerConnectionHandler.cpp
    src/doip/server/DoIpServerLogger.cpp
    src/doip/server/DoIpServerTransportConnection.cpp
    src/doip/server/DoIpServerTransportConnectionProvider.cpp
    src/doip/server/DoIpServerTransportLayer.cpp
    src/doip/server/DoIpServerTransportMessageHandler.cpp
    src/doip/server/DoIpServerVehicleIdentification.cpp
    src/doip/server/DoIpServerVehicleIdentificationSocketHandler.cpp)

target_include_directories(doip PUBLIC include)

target_link_libraries(
    doip
    PUBLIC async
           util
           common
           cpp2ethernet
           etl
           transport)

add_library(doipMock INTERFACE)

target_include_directories(doipMock INTERFACE mock/gmock/include)

target_link_libraries(doipMock INTERFACE doip gmock)
