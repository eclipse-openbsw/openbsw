add_library(udsFaultMgr src/udsFaultMgr/FaultManager.cpp
                        src/udsFaultMgr/InhibitionTable.cpp)

target_include_directories(udsFaultMgr PUBLIC include)

target_link_libraries(udsFaultMgr PUBLIC etl)
