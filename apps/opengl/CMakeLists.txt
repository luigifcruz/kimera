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