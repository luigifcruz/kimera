project(python)

find_package(pybind11 REQUIRED)

if(WINDOWS)
    set(PYBIND11_CPP_STANDARD /std:c++latest)
endif()

if(MACOS OR LINUX)
    set(PYBIND11_CPP_STANDARD -std=c++1z)
endif()

file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/index.cc")
pybind11_add_module(python ${SOURCE_LIST})

set_target_properties(python PROPERTIES OUTPUT_NAME kimera)
target_link_libraries(python PRIVATE kimera)
