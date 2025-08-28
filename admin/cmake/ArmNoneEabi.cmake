include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/ArmNoneEabi-header.cmake")

if (NOT DEFINED _ARCH_FLAGS)
    set(_ARCH_FLAGS "")
endif ()

set(_ARCH_FLAGS
    "${_ARCH_FLAGS} \
    -mcpu=cortex-m4 \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16 \
    -fmessage-length=0")

if (NOT DEFINED _CC_CXX_COMMON)
    set(_CC_CXX_COMMON "")
endif ()

set(_CC_CXX_COMMON
    "${_ARCH_FLAGS} \
    ${_CC_CXX_COMMON} \
    -Wno-psabi \
    -fdata-sections \
    -ffunction-sections \
    -fno-asynchronous-unwind-tables \
    -fno-builtin \
    -fno-common \
    -fshort-enums \
    -fstack-usage \
    -mno-unaligned-access \
    -mthumb")

if (NOT DEFINED _C_FLAGS)
    set(_C_FLAGS "")
endif ()

set(_C_FLAGS "${_CC_CXX_COMMON} ${_C_FLAGS} -ffreestanding")

if (NOT DEFINED _CXX_FLAGS)
    set(_CXX_FLAGS "")
endif ()

set(_CXX_FLAGS
    "${_CC_CXX_COMMON} \
    ${_CXX_FLAGS} \
    -fno-exceptions \
    -fno-non-call-exceptions \
    -fno-rtti \
    -fno-threadsafe-statics \
    -fno-use-cxa-atexit")

if (NOT DEFINED _EXE_LINKER_FLAGS)
    set(_EXE_LINKER_FLAGS "")
endif ()

set(_EXE_LINKER_FLAGS
    "${_ARCH_FLAGS} \
    ${_EXE_LINKER_FLAGS} \
    -static \
    -Wl,--gc-sections \
    -Wl,-Map,application.map,--cref")

set(_ASM_FLAGS "-g -mcpu=cortex-m4")

if (DEFINED CMAKE_CXX_FLAGS)
    if (NOT DEFINED _CXX_MERGED)
        set(_CXX_MERGED
            ON
            CACHE INTERNAL "")
        set(CMAKE_CXX_FLAGS
            "${_CXX_FLAGS} ${CMAKE_CXX_FLAGS}"
            CACHE STRING "C++ flags" FORCE)
    endif ()
else ()
    set(CMAKE_CXX_FLAGS_INIT "${_CXX_FLAGS}")
endif ()

if (DEFINED CMAKE_C_FLAGS)
    if (NOT DEFINED _C_MERGED)
        set(_C_MERGED
            ON
            CACHE INTERNAL "")
        set(CMAKE_C_FLAGS
            "${_C_FLAGS} ${CMAKE_C_FLAGS}"
            CACHE STRING "C flags" FORCE)
    endif ()
else ()
    set(CMAKE_C_FLAGS_INIT "${_C_FLAGS}")
endif ()

if (DEFINED CMAKE_ASM_FLAGS)
    if (NOT DEFINED _ASM_MERGED)
        set(_ASM_MERGED
            ON
            CACHE INTERNAL "")
        set(CMAKE_ASM_FLAGS
            "${_ASM_FLAGS} ${CMAKE_ASM_FLAGS}"
            CACHE STRING "C++ flags" FORCE)
    endif ()
else ()
    set(CMAKE_ASM_FLAGS_INIT "${_ASM_FLAGS}")
endif ()

if (DEFINED CMAKE_EXE_LINKER_FLAGS)
    if (NOT DEFINED _EXE_LINKER_MERGED)
        set(_EXE_LINKER_MERGED
            ON
            CACHE INTERNAL "")
        set(CMAKE_EXE_LINKER_FLAGS
            "${_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS}"
            CACHE STRING "linker flags" FORCE)
    endif ()
else ()
    set(CMAKE_EXE_LINKER_FLAGS_INIT "${_EXE_LINKER_FLAGS}")
endif ()
