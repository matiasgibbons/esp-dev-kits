set(UI_EEZ_TEST1_DIR ./ui_eez_test1/src/ui)
file(GLOB_RECURSE UI_EEZ_TEST1_SOURCES ${UI_EEZ_TEST1_DIR}/*.c ${UI_EEZ_TEST1_DIR}/*.cpp)

# WiFi app sources - usando GLOB para incluir todos los .c en app/
file(GLOB APP_SOURCES "app/*.c")

idf_component_register(
    SRCS "main.c" ${UI_EEZ_TEST1_SOURCES} ${APP_SOURCES}
    INCLUDE_DIRS "." ${UI_EEZ_TEST1_DIR} "app")

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-variable -Wno-format -DLV_LVGL_H_INCLUDE_SIMPLE -Wno-dangling-pointer -Wno-error=dangling-pointer)
target_compile_definitions(${COMPONENT_LIB} PRIVATE EEZ_FOR_LVGL LV_LVGL_H_INCLUDE_SIMPLE)