
pico_add_library(pico_scpi_usbtmc_dab)


target_sources(pico_unique_id INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/source/dab_utils.c
)

target_include_directories(pico_scpi_usbtmc_lablib INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include

)

target_link_libraries(pico_scpi_usbtmc_dab INTERFACE hardware_i2c)