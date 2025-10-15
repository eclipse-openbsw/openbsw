if (NOT BUILD_UNIT_TESTS)
    set(bspInterruptsImplName "bspInterruptsImpl")
else ()
    # For unit tests, the mock implementation from bspInterrupts is used. To
    # still be able to add tests for *this* module, the target is renamed.
    set(bspInterruptsImplName "bspInterruptsImplUt")
endif ()

add_library(${bspInterruptsImplName} INTERFACE)

target_include_directories(${bspInterruptsImplName} INTERFACE include)

target_link_libraries(${bspInterruptsImplName} INTERFACE platform)
