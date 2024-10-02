add_library(docan src/docan/common/DoCanLogger.cpp
                  src/docan/datalink/DoCanFrameCodecConfigPresets.cpp)

target_include_directories(docan PUBLIC include)

target_link_libraries(
    docan
    PUBLIC async
           bspInterrupts
           common
           cpp2can
           estd
           platform
           transport
           util)

if (BUILD_UNIT_TESTS)

    add_library(docanMock INTERFACE)

    target_include_directories(docanMock INTERFACE mock/gmock/include)

    target_link_libraries(docanMock INTERFACE docan)

endif ()
