add_library(asyncMock INTERFACE)

target_include_directories(asyncMock INTERFACE gmock/include)

target_link_libraries(asyncMock INTERFACE estd platform)

add_library(
    asyncMockImpl gmock/src/async/AsyncMock.cpp gmock/src/async/TestContext.cpp
                  gmock/src/async/Types.cpp)

target_include_directories(asyncMockImpl PRIVATE gmock/include)

target_link_libraries(asyncMockImpl PRIVATE async estd gmock platform)
