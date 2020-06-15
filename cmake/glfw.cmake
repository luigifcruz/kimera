project(glfw)

add_library(glfw INTERFACE IMPORTED GLOBAL)

if(LINUX OR MACOS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(glfw REQUIRED IMPORTED_TARGET glfw3)
endif()

if(WINDOWS)
    message(SEND_ERROR "GLFW isn't supported on Windows yet.")
endif()

target_include_directories(glfw INTERFACE ${glfw_INCLUDE_DIRS})
target_link_libraries(glfw INTERFACE ${glfw_LDFLAGS})

set_property(GLOBAL APPEND PROPERTY GLOBAL_LIB_LIST glfw)