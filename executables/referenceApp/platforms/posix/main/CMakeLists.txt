add_library(main src/main.cpp src/lifecycle/StaticBsp.cpp)

target_include_directories(main PRIVATE include)

target_link_libraries(main PRIVATE asyncBinding lifecycle bspEepromDriver)

add_library(osHooks src/osHooks.cpp)

target_link_libraries(osHooks PRIVATE freeRtos)

if (PLATFORM_SUPPORT_CAN)
    target_sources(main PRIVATE src/systems/CanSystem.cpp)

    target_link_libraries(main PRIVATE configuration socketCanTransceiver)
endif ()

if (PLATFORM_SUPPORT_ETHERNET)
    target_sources(main PRIVATE src/systems/TapEthernetSystem.cpp)
    target_link_libraries(main PRIVATE lwipSocket tapEthernetDriver
                                       ethConfiguration)

endif ()
