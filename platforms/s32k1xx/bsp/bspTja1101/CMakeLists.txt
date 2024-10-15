add_library(bspTja1101 src/phy/Tja1101.cpp src/phy/Tja1101Tester.cpp
                       src/mdio/MdioTja1101.cpp)

target_include_directories(bspTja1101 PUBLIC include)

target_link_libraries(bspTja1101 PUBLIC bspIo bspOutputManager bspConfiguration)
