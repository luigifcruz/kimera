project(glfw)

find_package(PkgConfig REQUIRED)
pkg_check_modules(glfw REQUIRED IMPORTED_TARGET glfw3)

add_library(glfw INTERFACE IMPORTED GLOBAL)
target_include_directories(glfw INTERFACE ${glfw_INCLUDE_DIRS})
target_link_libraries(glfw INTERFACE ${glfw_LDFLAGS})