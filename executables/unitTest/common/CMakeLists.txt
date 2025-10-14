add_library(utCommon src/util/EcuIdList.cpp src/util/crc/CRCCalculator.cpp)

target_include_directories(utCommon PUBLIC include)

target_link_libraries(utCommon PUBLIC util etl platform)
