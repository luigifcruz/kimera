find_package(PythonInterp REQUIRED)

if(LINUX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(VK REQUIRED QUIET IMPORTED_TARGET vulkan)
endif()

if(WINDOWS)
endif()

if(MACOS)
    set(VK_LDFLAGS "-lvulkan")
endif()

if(NOT VK_LDFLAGS)
    message(STATUS "Cannot find Vulkan dependencies. Ignoring it...")
    return()
endif()

target_include_directories(kimera PUBLIC ${VK_INCLUDE_DIRS})
target_link_libraries(kimera PUBLIC ${VK_LDFLAGS})

file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
target_sources(kimera PRIVATE ${SOURCE_LIST})

message(STATUS "Adding Vulkan support!")
SET(VULKAN_BACKEND_AVAILABLE ON CACHE BOOL "Enabling Vulkan backend" FORCE)
