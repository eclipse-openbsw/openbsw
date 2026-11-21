add_library(
    udsEventMgr
    src/udsEventMgr/DebounceCounter.cpp src/udsEventMgr/DebounceImmediate.cpp
    src/udsEventMgr/DebounceTimeBased.cpp src/udsEventMgr/EventManager.cpp)

target_include_directories(udsEventMgr PUBLIC include)

target_link_libraries(udsEventMgr PUBLIC etl udsFaultMgr)
