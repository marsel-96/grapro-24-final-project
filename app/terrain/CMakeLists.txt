file(GLOB_RECURSE target_inc "*.h" )
file(GLOB_RECURSE target_src "*.cpp" )

file(GLOB_RECURSE shaders "*.vert" "*.frag" "*.geom" "*.glsl")
source_group("Shaders" FILES ${shaders})
add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)

add_executable(${TARGET_NAME} ${target_inc} ${target_src} ${shaders})
target_link_libraries(${TARGET_NAME} PRIVATE itugl)
