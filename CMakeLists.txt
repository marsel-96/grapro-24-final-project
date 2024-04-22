cmake_minimum_required(VERSION 3.28)

# ---------------------------------------------------------------------------------
# Projects Settings
# ---------------------------------------------------------------------------------

# Setup vcpkg script with CMake (note: should be placed before project() call)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/_submodules/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

macro(subdir_list result curdir)
    file(GLOB children RELATIVE ${curdir} ${curdir}/*)
    set(dirlist "")
    foreach (child ${children})
        if (IS_DIRECTORY ${curdir}/${child})
            list(APPEND dirlist ${child})
        endif ()
    endforeach ()
    set(${result} ${dirlist})
endmacro()

# ---------------------------------------------------------------------------------
# Cmake projects
# ---------------------------------------------------------------------------------
set(TARGET_NAME grapro-24-final-project)

project(${TARGET_NAME})

find_package(Stb REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(FBX_SUPPORT OFF)

add_subdirectory(${CMAKE_SOURCE_DIR}/app)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib)