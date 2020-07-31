# Find OpenSSL Dependencies
if(LINUX OR MACOS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(OPENSSL REQUIRED IMPORTED_TARGET openssl)
endif()

if(WINDOWS)
endif()

target_include_directories(kimera PUBLIC ${OPENSSL_INCLUDE_DIRS})
target_link_libraries(kimera PUBLIC ${OPENSSL_LDFLAGS})

# Add module source list
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
target_sources(kimera PRIVATE ${SOURCE_LIST})
