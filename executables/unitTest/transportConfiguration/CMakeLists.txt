# transport is not configured in s32k1xx-unitTest mode (see libs/bsw/CMakeLists.txt).
# Skip this target there to keep compile_commands.json clean for clang-tidy.
if (NOT TARGET transport)
    return()
endif ()

add_library(transportConfiguration src/transport/TransportConfiguration.cpp)

target_include_directories(transportConfiguration PUBLIC include)

target_link_libraries(transportConfiguration PUBLIC transport etl)
