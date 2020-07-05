project(vk)

add_library(vk INTERFACE IMPORTED GLOBAL)

if(LINUX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(vk REQUIRED IMPORTED_TARGET vulkan)
endif()

target_include_directories(vk INTERFACE ${vk_INCLUDE_DIRS})
target_link_libraries(vk INTERFACE ${vk_LDFLAGS})

set_property(GLOBAL APPEND PROPERTY GLOBAL_LIB_LIST vk)
