add_library(safeLifecycle src/SafetyManager.cpp)

target_include_directories(safeLifecycle PUBLIC include)

target_link_libraries(safeLifecycle PRIVATE safeSupervisor safeUtils)
