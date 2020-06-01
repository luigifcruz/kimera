SUBDIRLIST(SUBDIRS "${kimera_SOURCE_DIR}/apps")

foreach(subdir ${SUBDIRS})
    add_subdirectory(${subdir})
endforeach()