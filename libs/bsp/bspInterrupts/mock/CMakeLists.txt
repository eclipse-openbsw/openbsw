add_library(bspInterruptsMock gmock/src/disableEnableAllInterruptsMock.cpp
                              gmock/src/SuspendResumeAllInterruptsMock.cpp)

target_include_directories(bspInterruptsMock PUBLIC gmock/include include)

target_link_libraries(bspInterruptsMock PUBLIC bspInterrupts etl gmock platform)

add_library(bspInterruptsImpl ALIAS bspInterruptsMock)
