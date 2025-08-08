add_library(lwipSysArch src/arch/sys_arch.cpp)

target_include_directories(lwipSysArch PUBLIC include)

target_link_libraries(lwipSysArch PRIVATE bsp)
