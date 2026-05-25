# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")
set(FREERTOS_KERNEL_PATH ${USERHOME}/.pico-sdk/FreeRTOS-Kernel)

set(TINYUSB_OPT_OS "OPT_OS_FREERTOS")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

set(PICO_EXTRAS_PATH ${USERHOME}/.pico-sdk/pico-extras)
add_subdirectory(${PICO_EXTRAS_PATH} pico_extras)

project(picow_ducky C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()

# Add executable. Default name is the project name, version 0.1
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR})

# Adding fatfs library
add_subdirectory(${USERHOME}/.pico-sdk/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src build)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/u8g2)

# Add executable. Default name is the project name, version 0.1

add_executable(picow_ducky)

if (DEFINED ENV{WIFI_SSID} AND (NOT WIFI_SSID))
        set(WIFI_SSID $ENV{WIFI_SSID})
        message("Using WIFI_SSID from environment ('${WIFI_SSID}')")
    endif()

    if (DEFINED ENV{WIFI_PASSWORD} AND (NOT WIFI_PASSWORD))
        set(WIFI_PASSWORD $ENV{WIFI_PASSWORD})
        message("Using WIFI_PASSWORD from environment")
    endif()

set(WIFI_SSID "${WIFI_SSID}" CACHE INTERNAL "WiFi SSID for examples")
set(WIFI_PASSWORD "${WIFI_PASSWORD}" CACHE INTERNAL "WiFi password for examples")


target_compile_definitions(picow_ducky 
    PUBLIC DEBUG=1
    portSUPPORT_SMP=1
    WIFI_SSID=\"${WIFI_SSID}\"
    WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
    # PICO_DEFAULT_UART_TX_PIN=12
    # PICO_DEFAULT_UART_RX_PIN=13
)

target_sources(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/picow_ducky.c 
        # ${CMAKE_CURRENT_LIST_DIR}/hw_config.c
        ${CMAKE_CURRENT_LIST_DIR}/src/middleware/sd_card.c
        ${CMAKE_CURRENT_LIST_DIR}/src/middleware/debug.c
        ${CMAKE_CURRENT_LIST_DIR}/src/middleware/input.c
        ${CMAKE_CURRENT_LIST_DIR}/src/hal/hal_gpio.c
        ${CMAKE_CURRENT_LIST_DIR}/src/hal/hal_sd.c
        ${CMAKE_CURRENT_LIST_DIR}/src/hal/hal.c
        ${CMAKE_CURRENT_LIST_DIR}/src/hal/hal_power.c
        ${CMAKE_CURRENT_LIST_DIR}/src/hal/hal_sd_glue.c
        ${CMAKE_CURRENT_LIST_DIR}/src/hal/hal_display.c
        ${CMAKE_CURRENT_LIST_DIR}/src/hal/hal_battery.c
        ${CMAKE_CURRENT_LIST_DIR}/src/app/web_server/web_server.c
        ${CMAKE_CURRENT_LIST_DIR}/src/app/web_server/ssi.c
        ${CMAKE_CURRENT_LIST_DIR}/src/app/web_server/cgi.c
        ${CMAKE_CURRENT_LIST_DIR}/src/app/web_server/post.c
        ${CMAKE_CURRENT_LIST_DIR}/src/app/pubsub/pubsub.c
        ${CMAKE_CURRENT_LIST_DIR}/src/app/pubsub/pubsub_free.c
        ${CMAKE_CURRENT_LIST_DIR}/src/app/duckyscript/ducky_handler.c
        ${CMAKE_CURRENT_LIST_DIR}/src/app/duckyscript/ducky_parser.c
        ${CMAKE_CURRENT_LIST_DIR}/src/app/duckyscript/ducky.c
        ${CMAKE_CURRENT_LIST_DIR}/src/app/duckyscript/ducky_view.c
        ${CMAKE_CURRENT_LIST_DIR}/src/app/hid/usb_device.c
        ${CMAKE_CURRENT_LIST_DIR}/src/app/hid/usb_descriptors.c
        ${CMAKE_CURRENT_LIST_DIR}/src/app/hid/usb_hid.c
        ${CMAKE_CURRENT_LIST_DIR}/src/app/gui/gui.c
        ${CMAKE_CURRENT_LIST_DIR}/src/app/gui/menu_view.c
        ${CMAKE_CURRENT_LIST_DIR}/src/app/gui/ui_common.c
        ${CMAKE_CURRENT_LIST_DIR}/src/app/gui/view_manager.c
        ${CMAKE_CURRENT_LIST_DIR}/src/app/settings/settings.c
        ${CMAKE_CURRENT_LIST_DIR}/src/app/settings/settings_view.c
        ${CMAKE_CURRENT_LIST_DIR}/src/app/status_led/status_led.c
        ${CMAKE_CURRENT_LIST_DIR}/src/app/task_manager/task_manager.c
)

pico_set_program_name(picow_ducky "picow_ducky")
pico_set_program_version(picow_ducky "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(picow_ducky 1)
pico_enable_stdio_usb(picow_ducky 1)

# Add the standard include files to the build
target_include_directories(picow_ducky PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/src/app
        ${CMAKE_CURRENT_LIST_DIR}/src/app/web_server #lwipopts.h
        ${CMAKE_CURRENT_LIST_DIR}/src/app/hid #tusb_config.h
        ${CMAKE_CURRENT_LIST_DIR}/src/app/hid/layouts
        ${PICO_LWIP_CONTRIB_PATH}/apps/httpd

)

# Add the standard library to the build
target_link_libraries(picow_ducky
        pico_stdlib
        hardware_sleep
        pico_lwip_http
        pico_lwip_mdns
        pico_freertos_httpd_content
        pico_cyw43_arch_lwip_sys_freertos
        hardware_i2c
        hardware_adc
        FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
        no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
        tinyusb_device
        tinyusb_board
        u8g2
    )

pico_add_extra_outputs(picow_ducky)

pico_add_library(pico_freertos_httpd_content NOFLAG)
pico_set_lwip_httpd_content(pico_freertos_httpd_content INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/app/web_server/content/index.shtml
        ${CMAKE_CURRENT_LIST_DIR}/src/app/web_server/content/index.css
        )