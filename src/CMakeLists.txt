# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

idf_component_register(SRCS ${app_sources} 
  INCLUDE_DIRS "."
PRIV_REQUIRES u8g2 u8g2_esp32_hal esp_timer idf::driver 
)

# Register this directory as the main component
#idf_component_register(SRCS "${main_sources}" "${main_headers}"
 #                   INCLUDE_DIRS "."
  #                  PRIV_REQUIRES u8g2 u8g2_esp32_hal esp_timer idf::driver 
#)