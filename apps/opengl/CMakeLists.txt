file(GLOB SOURCE_LIST CONFIGURE_DEPENDS *.c render/glad/*.c)

include(../../cmake/x11.cmake)
link_libraries("-lm -ldl -lGLESv2 -lEGL -lX11 -lglfw")
add_executable(opengl ${SOURCE_LIST})

set_target_properties(opengl PROPERTIES OUTPUT_NAME opengl)

target_link_libraries(opengl PRIVATE kimera)