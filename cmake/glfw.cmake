project(glfw)

add_library(glfw INTERFACE IMPORTED GLOBAL)

if(LINUX OR MACOS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(glfw REQUIRED IMPORTED_TARGET glfw3)
endif()

if(WINDOWS)
    file(GLOB glfw_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/deps/glfw*/include/")
    file(GLOB glfw_LDFLAGS "${CMAKE_SOURCE_DIR}/deps/glfw*/lib-vc2019/glfw3dll.lib")
endif()

target_include_directories(glfw INTERFACE ${glfw_INCLUDE_DIRS})
target_link_libraries(glfw INTERFACE ${glfw_LDFLAGS})

set_property(GLOBAL APPEND PROPERTY GLOBAL_LIB_LIST glfw)