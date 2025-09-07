set(UI_EEZ_TEST1_DIR ./ui_eez_test1/src/ui)
file(GLOB_RECURSE UI_EEZ_TEST1_SOURCES ${UI_EEZ_TEST1_DIR}/*.c ${UI_EEZ_TEST1_DIR}/*.cpp)

idf_component_register(
    SRCS "main.c" ${UI_EEZ_TEST1_SOURCES}
    INCLUDE_DIRS "." ${UI_EEZ_TEST1_DIR}
    PRIV_REQUIRES 
        esp32_s3_lcd_ev_board
        haruny__eez-framework
        lvgl__lvgl)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-variable -Wno-format -DLV_LVGL_H_INCLUDE_SIMPLE -Wno-dangling-pointer -Wno-error=dangling-pointer)
target_compile_definitions(${COMPONENT_LIB} PRIVATE EEZ_FOR_LVGL)

# Also apply EEZ_FOR_LVGL to the EEZ Framework component
idf_component_get_property(eez_lib haruny__eez-framework COMPONENT_LIB)
target_compile_definitions(${eez_lib} PRIVATE EEZ_FOR_LVGL LV_LVGL_H_INCLUDE_SIMPLE)
target_compile_options(${eez_lib} PRIVATE -Wno-dangling-pointer -Wno-error=dangling-pointer)
