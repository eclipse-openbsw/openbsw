set(OPENBSW_PLATFORM posix)

if (NOT CMAKE_SYSTEM_NAME OR NOT CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(PLATFORM_SUPPORT_IO
        OFF
        CACHE BOOL "Turn IO support on or off" FORCE)
endif ()
