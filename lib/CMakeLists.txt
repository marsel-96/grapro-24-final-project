# obtain the list of subdirectories
subdir_list(SUBDIRS ${CMAKE_CURRENT_LIST_DIR})

foreach (subdir ${SUBDIRS})
    add_subdirectory(${subdir})
endforeach ()
