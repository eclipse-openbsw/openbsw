add_library(routingUtils INTERFACE)

target_include_directories(routingUtils INTERFACE include)

target_link_libraries(routingUtils INTERFACE blob etl io lwipSocket)

add_library(routingPduConversion src/routing/PduConversion.cpp)

target_include_directories(routingPduConversion PUBLIC include)

target_link_libraries(routingPduConversion PUBLIC cpp2can etl)
