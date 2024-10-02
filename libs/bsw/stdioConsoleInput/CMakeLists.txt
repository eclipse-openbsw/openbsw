add_library(stdioConsoleInput src/console/StdioConsoleInput.cpp)

target_include_directories(stdioConsoleInput PUBLIC include)

target_link_libraries(stdioConsoleInput PUBLIC util)
