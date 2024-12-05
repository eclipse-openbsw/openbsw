add_library(
    bspInputManager
    src/inputManager/AlternativeDigitalInput.cpp
    src/inputManager/DigitalInput.cpp src/inputManager/DigitalInputTester.cpp)

target_include_directories(bspInputManager PUBLIC include)

target_link_libraries(
    bspInputManager
    PUBLIC bsp
           bspDynamicClient
           bspIo
           estd
           platform
           util)
