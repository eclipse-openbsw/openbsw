# The etl clock hooks are provided by the platform's SystemTimer, so only the
# print hook is implemented here.
add_library(etlImpl src/print.cpp)

target_link_libraries(etlImpl PRIVATE etl bsp util)
