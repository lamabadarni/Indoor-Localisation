file(GLOB_RECURSE main_sources "*.cpp" "*.c")
# Find all .h header files recursively (for dependency tracking)
file(GLOB_RECURSE main_headers "*.h")

# Register this directory as the main component
idf_component_register(SRCS "${main_sources}" "${main_headers}"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_lcd lvgl esp_timer idf::driver
)