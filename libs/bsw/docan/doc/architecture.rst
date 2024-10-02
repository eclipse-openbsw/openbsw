docan - Architecture
====================

.. uml::

   component DoCanTransportLayerContainer
   component DoCanTransportLayer
   component DoCanReceiver
   component DoCanTransmitter
   component DoCanPhysicalCanTransceiver
   component DoCanMessageReceiver
   component DoCanMessageReceiveProtocolHandler
   component DoCanMessageTransmitter
   component DoCanMessageTransmitProtocolHandler

   DoCanTransportLayerContainer -- DoCanTransportLayer
   DoCanTransportLayer -- DoCanReceiver
   DoCanTransportLayer -- DoCanTransmitter
   DoCanTransportLayer -- DoCanPhysicalCanTransceiver
   DoCanReceiver -- DoCanMessageReceiver
   DoCanMessageReceiver -- DoCanMessageReceiveProtocolHandler
   DoCanTransmitter -- DoCanMessageTransmitter
   DoCanMessageTransmitter -- DoCanMessageTransmitProtocolHandler
