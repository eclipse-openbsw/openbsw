add_library(bspOutputPwm src/outputPwm/OutputPwm.cpp
                         src/outputPwm/OutputPwmTester.cpp)

target_include_directories(bspOutputPwm PUBLIC include)

target_link_libraries(
    bspOutputPwm
    bspDynamicClient
    bspIo
    etl
    util)
