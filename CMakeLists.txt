cmake_minimum_required(VERSION 3.13)

include(/home/baruch/pico/pico-sdk/pico_sdk_init.cmake)

set(CMAKE_EXE_LINKER_FLAGS "-Wl,--print-memory-usage")
set(PROJECT_NAME picow_ducky)
set(PICO_BOARD pico_w)

project(${PROJECT_NAME})
enable_language( C CXX ASM )

pico_sdk_init()

# Adding fatfs library
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../pico/no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build)

add_executable(${PROJECT_NAME} main.c)

target_sources(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/lib/hid/usb_descriptors.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/hid/keyboard.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/duckyscript/parser.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/duckyscript/handler.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/sd_memory/sd_memory.c
)

# For TinyUSB can find tusb_config.h
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lib/hid/)

target_link_libraries(${PROJECT_NAME} PRIVATE 
                        pico_stdlib 
                        tinyusb_device 
                        tinyusb_board 
                        hardware_flash 
                        FatFs_SPI )


pico_add_extra_outputs(${PROJECT_NAME})