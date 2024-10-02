add_library(bspInterrupts INTERFACE)

target_include_directories(bspInterrupts INTERFACE include)

target_link_libraries(bspInterrupts INTERFACE bspInterruptsImpl)

if (BUILD_UNIT_TESTS)

    add_subdirectory(mock)

endif ()
