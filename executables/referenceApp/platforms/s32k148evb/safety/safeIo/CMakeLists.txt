add_library(safeIo src/safeIo/SafeIo.cpp src/safeIo/SafeState.cpp)

target_include_directories(safeIo PUBLIC include)

target_link_libraries(safeIo PRIVATE safeSupervisor bspIo)
