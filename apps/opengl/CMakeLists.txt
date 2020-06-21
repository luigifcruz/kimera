set(LIBS kimera glfw egl)

if(UNIX AND NOT APPLE)
    list(APPEND LIBS dl)
endif()

if(UNIX)
    set (CMAKE_C_STANDARD_LIBRARIES "-lm")
endif()

file(GLOB SOURCE_LIST CONFIGURE_DEPENDS *.c render/glad/*.c)

include(${CMAKE_SOURCE_DIR}/cmake/glfw.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/egl.cmake)

add_executable(opengl ${SOURCE_LIST})

target_link_libraries(opengl PRIVATE ${LIBS})

find_package(PythonInterp REQUIRED)

set(SHADER_COMPILER_BIN ${CMAKE_SOURCE_DIR}/utils/compile_shaders.py)
set(SHADER_COMPILER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/render/shaders)
set(SHADER_COMPILER_DST ${CMAKE_CURRENT_SOURCE_DIR}/render)

add_custom_target(
    shader_compiler ALL
    COMMAND ${PYTHON_EXECUTABLE} ${SHADER_COMPILER_BIN} ${SHADER_COMPILER_SRC} ${SHADER_COMPILER_DST}
    BYPRODUCTS enums.h
    COMMENT "Generating Shader Header"
)

add_dependencies(opengl shader_compiler)