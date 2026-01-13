add_library(loggerIntegration src/logger/LoggerComposition.cpp
                              src/logger/LoggerTime.cpp)

target_include_directories(loggerIntegration PUBLIC include)

target_link_libraries(loggerIntegration PRIVATE logger bspInterrupts etl
                                                etlImpl)
