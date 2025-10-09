add_library(middlewareHeaders INTERFACE)

target_include_directories(
    middlewareHeaders
    INTERFACE $<INSTALL_INTERFACE:include>
              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_link_libraries(middlewareHeaders INTERFACE etl)
