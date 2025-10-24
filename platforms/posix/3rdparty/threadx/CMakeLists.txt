cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

# Set up the project
project(threadXLinux
    LANGUAGES C ASM
)

# Define our target library and an alias for consumers
add_library(threadXLinux)

add_subdirectory(ports/linux/gnu)

add_library(threadXLinuxPort INTERFACE)

target_include_directories(threadXLinuxPort INTERFACE ports/linux/gnu/inc)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(
    threadXLinux PUBLIC bspInterrupts threadX threadXCoreConfiguration
                        Threads::Threads)
target_link_libraries(
    threadXLinuxPort INTERFACE threadXCoreConfiguration Threads::Threads)

target_compile_definitions(threadXLinuxPort INTERFACE TX_INCLUDE_USER_DEFINE_FILE)
target_compile_definitions(threadXLinux PRIVATE TX_INCLUDE_USER_DEFINE_FILE)

target_compile_options(threadXLinux PRIVATE --warn-no-unused-result)
