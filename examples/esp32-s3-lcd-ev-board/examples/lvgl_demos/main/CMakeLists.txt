# Definir rutas base
set(VIEW_DIR ${CMAKE_CURRENT_LIST_DIR}/view/src_ui)
set(CONTROLLER_DIR ${CMAKE_CURRENT_LIST_DIR}/controller)
set(MODEL_DIR ${CMAKE_CURRENT_LIST_DIR}/model)

# Buscar fuentes
file(GLOB_RECURSE VIEW_SOURCES ${VIEW_DIR}/*.c ${VIEW_DIR}/*.cpp)
file(GLOB CONTROLLER_SOURCES ${CONTROLLER_DIR}/*.c)
file(GLOB MODEL_SOURCES ${MODEL_DIR}/*.c)

idf_component_register(
    SRCS "main.c" ${VIEW_SOURCES} ${CONTROLLER_SOURCES} ${MODEL_SOURCES}
    INCLUDE_DIRS "." ${VIEW_DIR} "controller" "model"
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-variable -Wno-format -DLV_LVGL_H_INCLUDE_SIMPLE -Wno-dangling-pointer -Wno-error=dangling-pointer)
target_compile_definitions(${COMPONENT_LIB} PRIVATE EEZ_FOR_LVGL LV_LVGL_H_INCLUDE_SIMPLE)