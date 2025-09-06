set(LV_DEMO_DIR ../managed_components/lvgl__lvgl/demos)
file(GLOB_RECURSE LV_DEMOS_SOURCES ${LV_DEMO_DIR}/*.c)

set(UI_PRINTER_DIR ./ui_printer)
file(GLOB_RECURSE UI_PRINTER_SOURCES ${UI_PRINTER_DIR}/*.c)

set(UI_TUNER_DIR ./ui_tuner)
file(GLOB_RECURSE UI_TUNER_SOURCES ${UI_TUNER_DIR}/*.c)

set(UI_TXL_TEST1_DIR ./ui_txl_test1)
file(GLOB_RECURSE UI_TXL_TEST1_SOURCES ${UI_TXL_TEST1_DIR}/*.c)

idf_component_register(
    SRCS "main.c" ${LV_DEMOS_SOURCES} ${UI_PRINTER_SOURCES} ${UI_TUNER_SOURCES} ${UI_TXL_TEST1_SOURCES}
    INCLUDE_DIRS "." ${LV_DEMO_DIR} ${UI_PRINTER_DIR}/include ${UI_TUNER_DIR}/include ${UI_TXL_TEST1_DIR})

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-variable -Wno-format -DLV_LVGL_H_INCLUDE_SIMPLE)
