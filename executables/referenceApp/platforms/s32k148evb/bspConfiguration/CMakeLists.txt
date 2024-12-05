add_library(
    bspConfiguration
    src/bsp/adc/AnalogInput.cpp src/bsp/io/outputPwm/PwmSupport.cpp
    src/bsp/stdIo/stdIo.cpp src/bsp/SystemTimer/SystemTimer.cpp)

target_include_directories(bspConfiguration PUBLIC include)

target_link_libraries(
    bspConfiguration
    PUBLIC bspAdc
           bspCharInputOutput
           bspFtm
           bspFtmPwm
           bspInterrupts
           bspMcu
           bspOutputPwm)
