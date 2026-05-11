add_library(configuration common/src/busid/BusId.cpp)

target_include_directories(configuration PUBLIC include)

target_link_libraries(configuration PUBLIC commonHeaders async)

target_link_libraries(configuration INTERFACE common etl)

add_library(commonImpl common/src/busid/BusId.cpp)

target_link_libraries(commonImpl PRIVATE configuration async)

# python -m blob binary -i routing.jsonl -c blob.routing.table | python -m blob
# header data -n CONFIGURATION_BLOB > include/blob/configuration.h

# python -m blob.routing header ${BLOB_INPUT_FILE} > include/routing/channelId.h

# python -m blob header config-type > include/blob/ConfigType.h
target_include_directories(blobConfiguration INTERFACE include)
target_include_directories(routingConfiguration INTERFACE include)
