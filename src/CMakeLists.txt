configure_file(
    "${PROJECT_SOURCE_DIR}/include/kimera/state.hpp.in"
    "${PROJECT_SOURCE_DIR}/include/kimera/state.hpp"
)

add_library(kimera SHARED)
target_include_directories(kimera PUBLIC "${PROJECT_SOURCE_DIR}/include")

add_subdirectory(codec)
add_subdirectory(transport)
add_subdirectory(loopback)
add_subdirectory(client)

install(TARGETS kimera
        LIBRARY DESTINATION lib
        INCLUDES DESTINATION include)
