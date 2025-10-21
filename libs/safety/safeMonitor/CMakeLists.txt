add_library(safeMonitor INTERFACE)

target_include_directories(safeMonitor INTERFACE include)

target_link_libraries(safeMonitor INTERFACE platform common)
