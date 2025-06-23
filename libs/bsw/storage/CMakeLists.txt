set(storage.extraSources)

if (BUILD_UNIT_TESTS)
    add_library(storageMock INTERFACE)

    target_include_directories(storageMock INTERFACE mock/include)

    target_link_libraries(storageMock INTERFACE storage gmock_main)
else ()
    list(APPEND storage.extraSources src/storage/StorageTester.cpp)
endif ()

add_library(storage
    src/storage/MappingStorage.cpp
    src/storage/QueuingStorage.cpp
    src/storage/EepStorage.cpp
    ${storage.extraSources}
)

target_include_directories(storage PUBLIC include)

target_link_libraries(storage PUBLIC etl async asyncFreeRtosImpl util bsp)
