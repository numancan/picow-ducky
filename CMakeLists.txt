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
set(sdkVersion 2.3.0)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")
set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/FreeRTOS-Kernel)
set(TINYUSB_OPT_OS "OPT_OS_FREERTOS")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

set(PICO_EXTRAS_PATH ${USERHOME}/.pico-sdk/pico-extras)
add_subdirectory(${PICO_EXTRAS_PATH} pico_extras)

project(picozap C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR})

# Third-party libraries
add_subdirectory(lib/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src build)
add_subdirectory(lib/u8g2)

add_executable(${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME} PRIVATE
    DEBUG=1
    NO_SYS=0
    CYW43_ENABLE_BLUETOOTH=1
    NO_PICO_LED=1
    DEVICE_NAME=\"${PROJECT_NAME}\"
)

# Source paths are relative to this (top-level) CMakeLists, i.e. the project root
target_sources(${PROJECT_NAME} PRIVATE
        picozap.c

        src/config/task_config.c

        src/middleware/config_kv.c
        src/middleware/fat_io.c
        src/middleware/input.c
        src/middleware/radio_manager.c
        src/middleware/sleep_manager.c

        src/hal/hal.c
        src/hal/hal_gpio.c
        src/hal/hal_sd.c
        src/hal/hal_display.c
        src/hal/hal_battery.c

        src/app/web_server/web_server.c
        src/app/web_server/ssi.c
        src/app/web_server/cgi.c
        src/app/web_server/post.c

        src/app/duckyscript/ducky_handler.c
        src/app/duckyscript/ducky_parser.c
        src/app/duckyscript/ducky.c
        src/app/duckyscript/ducky_view.c
        src/app/duckyscript/ducky_settings.c
        src/app/duckyscript/ducky_settings_view.c
        src/app/duckyscript/payload.c
        src/app/duckyscript/web/ducky_cgi.c
        src/app/duckyscript/web/ducky_ssi.c
        src/app/duckyscript/web/ducky_post.c

        src/app/hid/usb_device.c
        src/app/hid/usb_descriptors.c
        src/app/hid/hid_report.c
        src/app/hid/layouts/hid_layout.c
        src/app/hid/hid_transport.c
        src/app/hid/hid_usb.c
        src/app/hid/hid_ble.c

        src/app/gui/gui.c
        src/app/gui/view.c
        src/app/gui/view_manager.c
        src/app/gui/canvas.c
        src/app/gui/elements.c
        src/app/gui/modules/menu.c

        src/app/power_manager/power_manager.c
        src/app/power_manager/sleep_view.c
        src/app/power_manager/battery_view.c

        src/app/net_manager/net_manager.c
        src/app/net_manager/net_manager_view.c
        src/app/net_manager/net_config.c
        src/app/net_manager/mdns_responder/mdns_responder.c
        src/app/net_manager/wifi_scan_list.c
        src/app/net_manager/dhcpserver/dhcpserver.c
        src/app/net_manager/dnsserver/dnsserver.c
        src/app/net_manager/web/provisioning_post.c
        src/app/net_manager/web/provisioning_ssi.c
        src/app/net_manager/web/provisioning_cgi.c
)

pico_set_program_name(${PROJECT_NAME} "${PROJECT_NAME}")
pico_set_program_version(${PROJECT_NAME} "2.0")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(${PROJECT_NAME} 1)
pico_enable_stdio_usb(${PROJECT_NAME} 0)

pico_btstack_make_gatt_header(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/app/hid/hog_combo.gatt")

target_include_directories(${PROJECT_NAME} PRIVATE
        .
        src
        src/config
        src/app
        src/app/duckyscript
        src/app/web_server
        src/app/hid
        src/app/hid/layouts
        src/middleware
        ${PICO_LWIP_CONTRIB_PATH}/apps/httpd
)

target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        pico_low_power
        hardware_watchdog
        # hardware_sleep
        # hardware_timer
        pico_lwip_http
        pico_lwip_mdns
        pico_freertos_httpd_content
        pico_cyw43_arch_lwip_sys_freertos
        hardware_i2c
        hardware_adc
        no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
        FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
        tinyusb_device
        tinyusb_board
        u8g2
        pico_btstack_ble
        pico_btstack_cyw43
)

pico_configure_ip4_address(${PROJECT_NAME} PRIVATE DEVICE_IP_AP_ADDRESS "192.168.4.1")
pico_configure_ip4_address(${PROJECT_NAME} PRIVATE DEVICE_IP_AP_MASK "255.255.255.0")

pico_add_extra_outputs(${PROJECT_NAME})

# Web content is generated at build time: tools/gen_web_content.py scans headers for
# `// $EXPORT=` markers and injects those values into a copy of the content tree, so the
# served files stay in sync with the C definitions (see the script for the marker format).
find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(WEB_CONTENT_SRC ${CMAKE_CURRENT_LIST_DIR}/src/app/web_server/content)
set(WEB_CONTENT_OUT ${CMAKE_CURRENT_BINARY_DIR}/web_content)
file(GLOB_RECURSE WEB_EXPORT_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/src/*.h)
file(GLOB_RECURSE WEB_CONTENT_SRC_FILES CONFIGURE_DEPENDS ${WEB_CONTENT_SRC}/*)

set(WEB_CONTENT_OUT_FILES
        ${WEB_CONTENT_OUT}/index.html
        ${WEB_CONTENT_OUT}/portal.html
        ${WEB_CONTENT_OUT}/404.html
        ${WEB_CONTENT_OUT}/css/style.css
        ${WEB_CONTENT_OUT}/api/ducky/payloads.shtml
        ${WEB_CONTENT_OUT}/api/ducky/settings.shtml
        ${WEB_CONTENT_OUT}/api/ducky/status.shtml
        ${WEB_CONTENT_OUT}/api/portal/status.shtml
        ${WEB_CONTENT_OUT}/api/portal/scan.shtml
        )

add_custom_command(
        OUTPUT ${WEB_CONTENT_OUT_FILES}
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/tools/gen_web_content.py
                --content-src ${WEB_CONTENT_SRC}
                --content-out ${WEB_CONTENT_OUT}
                --src-root ${CMAKE_CURRENT_LIST_DIR}/src
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/tools/gen_web_content.py
                ${WEB_CONTENT_SRC_FILES}
                ${WEB_EXPORT_HEADERS}
        VERBATIM)

pico_add_library(pico_freertos_httpd_content NOFLAG)
pico_set_lwip_httpd_content(pico_freertos_httpd_content INTERFACE ${WEB_CONTENT_OUT_FILES})
