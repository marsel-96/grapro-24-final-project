set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE target_inc "*.h" )
file(GLOB_RECURSE target_src "*.cpp" )
file(GLOB_RECURSE target_mod "*.ixx" )

file(GLOB_RECURSE shaders "*.vert" "*.frag" "*.geom" "*.glsl" "*.tesc" "*tese" "*comp")
source_group("Shaders" FILES ${shaders})

add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)
add_executable(${TARGET_NAME} ${target_inc} ${target_src} ${shaders})
target_sources(${TARGET_NAME}
        PUBLIC
        FILE_SET CXX_MODULES FILES ${target_mod})

target_precompile_headers(${TARGET_NAME} PRIVATE src/_pch/_pch.h)
target_include_directories(${TARGET_NAME} PRIVATE src/_pch src/include)

target_link_libraries(${TARGET_NAME} PRIVATE itugl)
