add_library(core_headers INTERFACE)

target_include_directories(
    core_headers
    INTERFACE $<INSTALL_INTERFACE:include>
              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_link_libraries(core_headers INTERFACE etl)
