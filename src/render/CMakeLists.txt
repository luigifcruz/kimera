if(LINUX OR MACOS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW REQUIRED IMPORTED_TARGET glfw3)
    list(APPEND GLFW_LDFLAGS "-lm -lpthread")
endif()

if(LINUX)
    pkg_check_modules(X11 REQUIRED IMPORTED_TARGET x11)
endif()

if(WINDOWS)
endif()

target_include_directories(kimera PUBLIC ${GLFW_INCLUDE_DIRS})
target_link_libraries(kimera PUBLIC ${GLFW_LDFLAGS} ${X11_LDFLAGS})

add_subdirectory(opengl)
add_subdirectory(vulkan)

file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
target_sources(kimera PRIVATE ${SOURCE_LIST})
