cmake_minimum_required(VERSION 3.13)

include(/home/baruch/pico/pico-sdk/pico_sdk_init.cmake)

project(dev_hid_composite)

set(PICO_BOARD pico_w)

pico_sdk_init()

enable_language( C CXX ASM )

add_subdirectory(/home/baruch/pico/no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build)

add_executable(dev_hid_composite)

target_sources(dev_hid_composite PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/src/main.c
        ${CMAKE_CURRENT_LIST_DIR}/src/usb_descriptors.c
        ${CMAKE_CURRENT_LIST_DIR}/src/hid_keyboard.c
        ${CMAKE_CURRENT_LIST_DIR}/src/http_server.c
        ${CMAKE_CURRENT_LIST_DIR}/src/sd_memory.c
        ${CMAKE_CURRENT_LIST_DIR}/src/hw_config.c
        )



# Make sure TinyUSB can find tusb_config.h
target_include_directories(dev_hid_composite PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/inc)

# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_device
# for TinyUSB device support and tinyusb_board for the additional board support library used by the example
target_link_libraries(dev_hid_composite PUBLIC 
                        pico_stdlib 
                        tinyusb_device 
                        tinyusb_board 
                        hardware_flash 
                        hardware_sync
                        pico_cyw43_arch_none
                        pico_cyw43_arch_lwip_threadsafe_background 
                        pico_lwip_http
                        pico_multicore
                        FatFs_SPI )

# Uncomment this line to enable fix for Errata RP2040-E5 (the fix requires use of GPIO 15)
#target_compile_definitions(dev_hid_composite PUBLIC PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1)
pico_add_extra_outputs(dev_hid_composite)

# add url via pico_set_program_url
# example_auto_set_url(dev_hid_composite)
