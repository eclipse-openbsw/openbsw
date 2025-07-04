add_library(hardFaultHandler src/hardFaultHandler.s)

# is the dependency to startUp needed?
target_link_libraries(hardFaultHandler PRIVATE startUp)
