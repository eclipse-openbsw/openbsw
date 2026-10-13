add_library(hardFaultHandler src/hardFaultHandler.S)

target_link_libraries(hardFaultHandler PRIVATE bspMcu)
