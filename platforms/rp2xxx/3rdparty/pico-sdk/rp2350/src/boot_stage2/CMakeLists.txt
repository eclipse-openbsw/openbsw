add_executable(
    boot_stage2
    boot2_w25q080.S)

unset(CMAKE_ASM_FLAGS)
unset(CMAKE_CXX_FLAGS)
unset(CMAKE_C_FLAGS)
set(CMAKE_EXE_LINKER_FLAGS "-mcpu=cortex-m33 -mthumb -g -O3")

target_link_libraries(
    boot_stage2
    PRIVATE pico-interface)

target_compile_definitions(boot_stage2 PUBLIC
    LIB_BOOT_STAGE2_HEADERS=1
)

set_target_properties(
    boot_stage2
    PROPERTIES PROP_LINKER_SCRIPT boot_stage2.ld)

set_target_properties(
    boot_stage2
    PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/boot_stage2.ld)

target_link_options(boot_stage2 PRIVATE "-nostartfiles")
target_link_options(boot_stage2 PRIVATE "--specs=nosys.specs")
target_link_options(boot_stage2 PRIVATE -T ${CMAKE_CURRENT_LIST_DIR}/boot_stage2.ld)

add_custom_target(
    boot_stage2_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/boot_stage2.bin)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/boot_stage2.bin
    DEPENDS boot_stage2
    COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:boot_stage2> ${CMAKE_CURRENT_BINARY_DIR}/boot_stage2.bin
    VERBATIM)

# On the RP2350 the boot stage 2 is entered from crt0 rather than from the
# bootrom, so the image is only padded and not checksummed.
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/boot_stage2_checksummed.S
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/boot_stage2.bin
    COMMAND python3 ${CMAKE_CURRENT_LIST_DIR}/pad_checksum -s 0xffffffff -a arm ${CMAKE_CURRENT_BINARY_DIR}/boot_stage2.bin ${CMAKE_CURRENT_BINARY_DIR}/boot_stage2_checksummed.S
    VERBATIM)

add_library(boot_stage2_library OBJECT ${CMAKE_CURRENT_BINARY_DIR}/boot_stage2_checksummed.S)
