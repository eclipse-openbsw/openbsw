add_library(utCommon src/util/EcuIdList.cpp src/util/crc/CRCCalculator.cpp)

target_include_directories(utCommon PUBLIC include test/gtest/include)

target_link_libraries(utCommon PUBLIC util etl platform)

if (BUILD_UNIT_TESTS)
    add_library(utCommonMock INTERFACE)

    target_include_directories(utCommonMock INTERFACE mock/include)

endif ()
