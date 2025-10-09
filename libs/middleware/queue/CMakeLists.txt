add_library(queue INTERFACE)

target_include_directories(
    queue INTERFACE $<INSTALL_INTERFACE:include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_link_libraries(queue INTERFACE etl)
