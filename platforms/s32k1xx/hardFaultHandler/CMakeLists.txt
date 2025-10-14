add_library(hardFaultHandler src/hardFaultHandler.s)

set_target_properties(hardFaultHandler PROPERTIES COMPILE_WARNING_AS_ERROR OFF)

target_compile_options(
    hardFaultHandler
    PRIVATE "$<$<C_COMPILER_ID:Clang>:-Wno-unused-command-line-argument>")

target_link_libraries(hardFaultHandler PRIVATE)
