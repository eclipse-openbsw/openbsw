add_library(
    blob
    src/blob/Blob.cpp
    src/blob/Config.cpp
    src/blob/Header.cpp
    src/blob/Logger.cpp
    src/blob/util.cpp)

target_include_directories(blob PUBLIC include)

add_library(blobConfiguration INTERFACE)

target_link_libraries(blob PUBLIC blobConfiguration etl util)
