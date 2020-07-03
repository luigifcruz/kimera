project(openssl)

add_library(openssl INTERFACE IMPORTED GLOBAL)

if(LINUX OR MACOS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(OPENSSL REQUIRED IMPORTED_TARGET openssl)
endif()

if(WINDOWS)
endif()

target_include_directories(openssl INTERFACE ${OPENSSL_INCLUDE_DIRS})
target_link_libraries(openssl INTERFACE ${OPENSSL_LDFLAGS})

set_property(GLOBAL APPEND PROPERTY GLOBAL_LIB_LIST openssl)