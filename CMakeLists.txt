cmake_minimum_required(VERSION 3.12)

include(${PICO_SDK_PATH}/pico_sdk_init.cmake)

set(CMAKE_EXE_LINKER_FLAGS "-Wl,--print-memory-usage")
set(PROJECT_NAME picow_ducky)
set(PICO_BOARD pico_w)

project(${PROJECT_NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

pico_sdk_init()


# Adding fatfs library
add_subdirectory(${PICO_SDK_PATH}/../no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build)

# add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/webserver/)

add_executable(${PROJECT_NAME} main.c)

target_sources(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/lib/hid/usb_descriptors.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/hid/keyboard.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/duckyscript/parser.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/duckyscript/handler.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/sd_memory/sd_memory.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/webserver/webserver.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/webserver/post.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/webserver/ssi.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/settings/settings.c
)

# For TinyUSB can find tusb_config.h
# For lwIP can find lwipopts.h
target_include_directories(${PROJECT_NAME} PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR}/lib/
        ${CMAKE_CURRENT_LIST_DIR}/lib/hid/
        ${CMAKE_CURRENT_LIST_DIR}/lib/webserver/
        ${CMAKE_CURRENT_LIST_DIR}/lib/sd_memory/
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
                        pico_stdlib 
                        pico_platform
                        tinyusb_device 
                        tinyusb_board 
                        hardware_flash 
                        FatFs_SPI
                        pico_multicore
                        pico_cyw43_arch_lwip_threadsafe_background 
                        # pico_cyw43_arch_lwip_poll
                        pico_lwip_http)

pico_add_extra_outputs(${PROJECT_NAME})