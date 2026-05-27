add_library(u8g2 INTERFACE)
target_sources(u8g2 INTERFACE
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8log.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8log_u8g2.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8log_u8x8.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8x8_8x8.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8g2_bitmap.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8x8_byte.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8g2_box.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8x8_cad.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8g2_buffer.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8x8_capture.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8g2_button.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8g2_circle.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8g2_cleardisplay.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8x8_debounce.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8g2_d_memory.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8x8_d_ssd1306_128x32.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8x8_display.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8g2_d_setup.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8x8_d_ssd1306_128x64_noname.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8x8_fonts.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8g2_font.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8x8_gpio.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8g2_fonts.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8x8_input_value.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8g2_hvline.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8x8_message.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8g2_input_value.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8x8_selection_list.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8g2_intersection.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8x8_setup.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8g2_kerning.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8x8_string.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8g2_line.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8x8_u16toa.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8g2_ll_hvline.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8x8_u8toa.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8g2_message.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8g2_polygon.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8g2_selection_list.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8x8_d_stdio.c
               ${CMAKE_CURRENT_LIST_DIR}/csrc/u8g2_setup.c
			   )

target_include_directories(u8g2 INTERFACE csrc)


