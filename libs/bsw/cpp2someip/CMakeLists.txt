add_library(
    cpp2someip
    src/BufferedEventSender.cpp
    src/EtlString.cpp
    src/EventMessage.cpp
    src/EventTransceiver.cpp
    src/ITpTransceiver.cpp
    src/Network.cpp
    src/NetworkChannel.cpp
    src/NetworkConfig.cpp
    src/NetworkResource.cpp
    src/QueryManager.cpp
    src/RebootTracker.cpp
    src/RebootTrackerEndpoint.cpp
    src/RpcBaseClient.cpp
    src/RpcChannel.cpp
    src/RpcClosure.cpp
    src/RpcHandler.cpp
    src/RpcReceiver.cpp
    src/RpcServiceRegistry.cpp
    src/RpcSomeIpStack.cpp
    src/SdMessageBuilder.cpp
    src/SdMessageParser.cpp
    src/SdOptionParser.cpp
    src/SdOptions.cpp
    src/SdReceiver.cpp
    src/SdServiceRegistry.cpp
    src/SdSomeIpStack.cpp
    src/ServiceAnnouncer.cpp
    src/ServiceAnnouncerTask.cpp
    src/ServiceDescription.cpp
    src/ServiceHandler.cpp
    src/ServiceManager.cpp
    src/ServiceQuery.cpp
    src/ServiceTracker.cpp
    src/SessionManager.cpp
    src/SomeIpMessage.cpp
    src/SomeIpParser.cpp
    src/SomeIpSerializer.cpp
    src/SomeIpStack.cpp
    src/SomeIpStreamer.cpp
    src/Statistics.cpp
    src/StatisticsEvaluator.cpp
    src/SubscribedEventGroup.cpp
    src/SubscriptionEndpoint.cpp
    src/SubscriptionManager.cpp
    src/TcpClientChannelValidator.cpp
    src/TcpConfig.cpp
    src/TcpProxy.cpp
    src/TcpServer.cpp
    src/TpReceiver.cpp
    src/TpSender.cpp
    src/TpTransceiver.cpp
    src/UdpConfig.cpp
    src/UdpProxy.cpp
    src/defaultlogger.cpp
    src/logger.cpp)

target_include_directories(cpp2someip PUBLIC include)

target_link_libraries(cpp2someip PUBLIC etl async cpp2ethernet)

if (BUILD_EXECUTABLE STREQUAL "unitTest")
    add_library(cpp2someipMock INTERFACE)

    target_include_directories(cpp2someipMock INTERFACE mock/include)

    target_link_libraries(cpp2someipMock INTERFACE cpp2someip gmock)
endif ()
