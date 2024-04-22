subdir_list(SUBDIRS ${CMAKE_CURRENT_LIST_DIR})

foreach (subdir ${SUBDIRS})
	set(TARGET_NAME ${subdir})
    add_subdirectory(${subdir})
	set_target_properties(${TARGET_NAME} PROPERTIES
		FOLDER app/${subdir}
		VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/${subdir})
endforeach ()

