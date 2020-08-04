add_library(kimera SHARED)
target_include_directories(kimera PUBLIC "${PROJECT_SOURCE_DIR}/include")

option(OPENGL_BACKEND_AVAILABLE "Compile OpenGL Backend" OFF)
option(VULKAN_BACKEND_AVAILABLE "Compile Vulkan Backend" OFF)

add_subdirectory(state)
add_subdirectory(codec)
add_subdirectory(render)
add_subdirectory(transport)
add_subdirectory(loopback)
add_subdirectory(client)

# Attach flags to header file
configure_file(
    "${PROJECT_SOURCE_DIR}/include/kimera/state.hpp.in"
    "${PROJECT_SOURCE_DIR}/include/kimera/state.hpp"
)

install(TARGETS kimera
        LIBRARY DESTINATION lib
        INCLUDES DESTINATION include)
