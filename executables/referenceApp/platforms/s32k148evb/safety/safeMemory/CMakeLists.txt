add_library(
    safeMemory
    src/safeMemory/MemoryProtection.cpp
    src/safeMemory/SafeMemory.cpp
    src/safeMemory/IntegrityEccHandler.cpp
    src/safeMemory/RomCheck.cpp)

target_include_directories(safeMemory PUBLIC include)

target_link_libraries(
    safeMemory
    PUBLIC platform
           bspMcu
           bspInterrupts
           safeUtils
           safeSupervisor
           async
           configuration)
