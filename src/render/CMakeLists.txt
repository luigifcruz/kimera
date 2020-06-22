include(${CMAKE_SOURCE_DIR}/cmake/egl.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/glfw.cmake)

if(LINUX)
    list(APPEND LIB_LIST dl)
endif()

list(APPEND LIB_LIST glfw egl)
file(GLOB TMP_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
list(APPEND SOURCE_LIST ${TMP_LIST})

# Compile Shaders
find_package(PythonInterp REQUIRED)

set(SHADER_COMPILER_BIN ${CMAKE_SOURCE_DIR}/utils/compile_shaders.py)
set(SHADER_COMPILER_SRC ${PROJECT_SOURCE_DIR}/shaders)
set(SHADER_COMPILER_DST ${CMAKE_SOURCE_DIR}/include/kimera)

add_custom_target(
    shader_compiler ALL
    COMMAND ${PYTHON_EXECUTABLE} ${SHADER_COMPILER_BIN} ${SHADER_COMPILER_SRC} ${SHADER_COMPILER_DST}
    BYPRODUCTS enums.h
    COMMENT "Generating Shader Header"
)

# Append to Global
set_property(GLOBAL APPEND PROPERTY GLOBAL_DEPENDENCIES shader_compiler)
set_property(GLOBAL APPEND PROPERTY GLOBAL_SOURCE_LIST ${SOURCE_LIST})
set_property(GLOBAL APPEND PROPERTY GLOBAL_LIB_LIST ${LIB_LIST})