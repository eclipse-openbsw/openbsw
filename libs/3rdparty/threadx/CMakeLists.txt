cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

# Set up the project
project(threadX
    LANGUAGES C ASM
)

# Define our target library and an alias for consumers
add_library(${PROJECT_NAME})

add_subdirectory(common)

target_link_libraries(
    threadX
    PUBLIC platform threadXConfiguration threadXPort
    PRIVATE threadXPortImpl osHooks)

target_compile_options(
    threadX PRIVATE "$<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wno-error=unused-parameter>"
)
