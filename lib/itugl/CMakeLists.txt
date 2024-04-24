# Recursively include all header files
set(target_inc "")
file(GLOB subdirectories RELATIVE ${CMAKE_CURRENT_LIST_DIR}/include ${CMAKE_CURRENT_LIST_DIR}/include/*)
foreach (subdir ${subdirectories})
    file(GLOB_RECURSE subdir_inc "${CMAKE_CURRENT_LIST_DIR}/include/${subdir}/*.h")
    source_group(${subdir} FILES ${subdir_inc})
    list(APPEND target_inc ${subdir_inc})
endforeach ()

# Recursively include all source files
set(target_src "")
file(GLOB subdirectories RELATIVE ${CMAKE_CURRENT_LIST_DIR}/src ${CMAKE_CURRENT_LIST_DIR}/src/*)
foreach (subdir ${subdirectories})
    file(GLOB_RECURSE subdir_src "${CMAKE_CURRENT_LIST_DIR}/src/${subdir}/*.cpp")
    source_group(${subdir} FILES ${subdir_src})
    list(APPEND target_src ${subdir_src})
endforeach ()


# Define GLM_ENABLE_EXPERIMENTAL to enable experimental features
add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)

add_library(itugl STATIC ${target_inc} ${target_src} _pch/_pch.cpp _pch/_pch.h)
target_include_directories(itugl PUBLIC include)
target_precompile_headers(itugl PRIVATE _pch/_pch.h)
target_link_libraries(itugl
        PUBLIC glm::glm
        PUBLIC assimp::assimp
        PUBLIC glfw
        PUBLIC glad::glad
        PUBLIC imgui::imgui
)