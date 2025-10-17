add_library(main src/main.cpp src/lifecycle/StaticBsp.cpp)

target_include_directories(main PUBLIC include)

target_link_libraries(
    main
    PRIVATE asyncBinding lifecycle safeSupervisor
    PUBLIC bspEepromDriver)

if (BUILD_TARGET_RTOS STREQUAL "FREERTOS")
    add_library(osHooks src/osHooks/freertos/osHooks.cpp)
    target_link_libraries(osHooks PRIVATE freeRtos)
elseif (BUILD_TARGET_RTOS STREQUAL "THREADX")
    add_library(osHooks src/osHooks/threadx/osHooks.cpp)
    target_link_libraries(osHooks PRIVATE threadX)
endif ()

if (PLATFORM_SUPPORT_CAN)
    target_sources(main PRIVATE src/systems/CanSystem.cpp)

    target_link_libraries(main PRIVATE configuration socketCanTransceiver)
endif ()

if (PLATFORM_SUPPORT_ETHERNET)
    target_sources(main PRIVATE src/systems/TapEthernetSystem.cpp)
    target_link_libraries(main PRIVATE lwipSocket tapEthernetDriver
                                       ethConfiguration configuration)

endif ()
