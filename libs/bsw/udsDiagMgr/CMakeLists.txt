add_library(udsDiagMgr src/udsDiagMgr/DiagMgr.cpp)

target_include_directories(udsDiagMgr PUBLIC include)

target_link_libraries(udsDiagMgr PUBLIC udsEventMgr udsFaultMgr)
