add_library(safeMemory src/safeMemory/MemoryProtection.cpp)

target_include_directories(safeMemory PUBLIC include)

target_link_libraries(safeMemory PUBLIC common util platform safeLifecycle)

if (PLATFORM_SUPPORT_MPU)
    target_link_libraries(safeMemory PUBLIC bspMcu)
endif ()
