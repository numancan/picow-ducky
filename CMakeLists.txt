cmake_minimum_required(VERSION 3.12)

include(${PICO_SDK_PATH}/pico_sdk_init.cmake)

set(CMAKE_EXE_LINKER_FLAGS "-Wl,--print-memory-usage")

project(common C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})
# set(PICO_BOARD "pico")
pico_sdk_init()

# Adding fatfs library
add_subdirectory(${PICO_SDK_PATH}/../no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build)

add_library(common INTERFACE)

target_sources(common INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/hid/usb_descriptors.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/hid/keyboard.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/duckyscript/parser.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/duckyscript/handler.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/sd_memory/sd_memory.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/settings/settings.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/status_led/status_led.c
        )

target_include_directories(common INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/lib/
        ${CMAKE_CURRENT_LIST_DIR}/lib/hid/
        ${CMAKE_CURRENT_LIST_DIR}/lib/sd_memory/
        ${CMAKE_CURRENT_LIST_DIR}/lib/status_led/
        )

target_link_libraries(common INTERFACE 
        pico_stdlib 
        pico_platform
        tinyusb_device 
        tinyusb_board 
        hardware_flash 
        FatFs_SPI
        pico_multicore
        )

add_executable(pico_ducky)
target_link_libraries(pico_ducky common)
pico_add_extra_outputs(pico_ducky)

add_executable(picow_ducky)
target_sources(picow_ducky PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/lib/webserver/webserver.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/webserver/post.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/webserver/ssi.c
        )

target_include_directories(picow_ducky PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR}/lib/webserver/
        )

target_link_libraries(picow_ducky common
        pico_cyw43_arch_lwip_threadsafe_background 
        # pico_cyw43_arch_lwip_poll
        pico_lwip_http
        )

target_compile_definitions(picow_ducky PRIVATE
PICOW=1)

pico_add_extra_outputs(picow_ducky)