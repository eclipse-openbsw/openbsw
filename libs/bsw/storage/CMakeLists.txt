add_library(storage src/storage/MappingStorage.cpp src/storage/QueuingStorage.cpp src/storage/EepStorage.cpp)

target_include_directories(storage PUBLIC include)

target_link_libraries(storage PUBLIC etl async util bsp)

if (BUILD_UNIT_TESTS)
    add_library(storageMock INTERFACE)

    target_include_directories(storageMock INTERFACE mock/include)

    target_link_libraries(storageMock INTERFACE storage gmock_main)
endif ()
