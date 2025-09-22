add_library(safeSystem src/safeSystem/SafeSystem.cpp)

target_include_directories(safeSystem PUBLIC include)

target_link_libraries(
    safeSystem
    PUBLIC safeSupervisor
           safeMemory
           bspMcu
           bspInterrupts
           bspIo)
