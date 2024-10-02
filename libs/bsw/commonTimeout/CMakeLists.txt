add_library(commonTimeout src/util/timeout/AbstractTimeout.cpp
                          src/util/timeout/AsyncTimeoutManager.cpp)

target_include_directories(commonTimeout PUBLIC include)

target_link_libraries(commonTimeout PUBLIC async common estd)

if (BUILD_UNIT_TESTS)

    add_library(commonTimeoutMock INTERFACE)

    target_include_directories(commonTimeoutMock INTERFACE mock/include)

endif ()
