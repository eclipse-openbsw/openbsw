add_library(safeSupervisorMock src/safetyLogger.cpp
                               src/suspendResumeAllInterrupts.cpp)

target_include_directories(safeSupervisorMock PUBLIC include)

target_link_libraries(safeSupervisorMock PUBLIC util)
