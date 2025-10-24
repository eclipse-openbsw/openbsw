cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

# Set up the project
project(threadXCortexM4
    LANGUAGES C ASM
)

add_library(threadXCortexM4)

add_subdirectory(ports/cortex_m4/gnu)

add_library(threadXCortexM4Port INTERFACE)

target_include_directories(threadXCortexM4Port INTERFACE ports/cortex_m4/gnu/inc)

target_link_libraries(threadXCortexM4 PUBLIC bspInterrupts bspMcu threadX threadXCoreConfiguration)
target_link_libraries(threadXCortexM4Port INTERFACE threadXCoreConfiguration)

target_compile_definitions(threadXCortexM4Port INTERFACE TX_INCLUDE_USER_DEFINE_FILE)
target_compile_definitions(threadXCortexM4 PRIVATE TX_INCLUDE_USER_DEFINE_FILE)
