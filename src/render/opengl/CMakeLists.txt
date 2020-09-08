find_package(PythonInterp REQUIRED)

if(LINUX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(EGL REQUIRED QUIET IMPORTED_TARGET egl)
endif()

if(WINDOWS)
    file(GLOB EGL_LDFLAGS "${CMAKE_SOURCE_DIR}/deps/angle/*.lib")
endif()

if(MACOS)
    file(GLOB EGL_LDFLAGS "${CMAKE_SOURCE_DIR}/deps/angle/*.dylib")
endif()

if(NOT EGL_LDFLAGS)
    message(STATUS "Cannot find OpenGL dependencies. Ignoring it...")
    return()
endif()

target_include_directories(kimera PUBLIC ${EGL_INCLUDE_DIRS})
target_link_libraries(kimera PUBLIC ${EGL_LDFLAGS})

file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
file(GLOB FW_SOURCE_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/glad/*.c")
target_sources(kimera PRIVATE ${SOURCE_LIST} ${FW_SOURCE_LIST})

#set(SHADER_COMPILER_BIN ${CMAKE_SOURCE_DIR}/utils/compile_shaders.py)
#set(SHADER_COMPILER_SRC ${PROJECT_SOURCE_DIR}/shaders)
#set(SHADER_COMPILER_DST ${CMAKE_SOURCE_DIR}/include/kimera)

#add_custom_target(
#    shader_compiler ALL
#    COMMAND ${PYTHON_EXECUTABLE} ${SHADER_COMPILER_BIN} ${SHADER_COMPILER_SRC} ${SHADER_COMPILER_DST}
#    BYPRODUCTS enums.h
#    COMMENT "Generating Shader Header"
#)

message(STATUS "Adding OpenGL support!")
SET(OPENGL_BACKEND_AVAILABLE ON CACHE BOOL "Enable OpenGL backend" FORCE)
