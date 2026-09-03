cmake_minimum_required(VERSION 3.25)

project(gfx-research-base VERSION 0.2.4 LANGUAGES C CXX)

if(NOT DEFINED CPM_SOURCE_CACHE)
    set(CPM_SOURCE_CACHE "${CMAKE_SOURCE_DIR}/.cpm-cache" CACHE PATH "CPM source cache")
endif()

if(NOT COMMAND CPMAddPackage)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake")
endif()

set(GFX_RESEARCH_BASE_THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/thirdparty" CACHE PATH "Third-party source directory")

option(GFX_RESEARCH_BASE_BUILD_EXAMPLE "Build the gfx-research-base example" ${PROJECT_IS_TOP_LEVEL})
option(GFX_RESEARCH_STATIC_RUNTIME "Link the executable runtime statically on supported Windows toolchains" ON)

# Keep all third-party libraries static. The operating system OpenGL/Win32
# libraries remain normal system dependencies and do not need to be shipped.
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

function(gfx_research_configure_executable target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "gfx_research_configure_executable: unknown target '${target}'")
    endif()

    if(NOT GFX_RESEARCH_STATIC_RUNTIME OR NOT WIN32)
        return()
    endif()

    if(MSVC)
        set_property(TARGET ${target} PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
        )
    elseif(MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_options(${target} PRIVATE
            -static
            -static-libgcc
            -static-libstdc++
        )
    elseif(MINGW AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_link_options(${target} PRIVATE -static)
    endif()
endfunction()

# Dependencies are downloaded to stable source directories so IDE reloads do not
# recreate a large FetchContent tree on every clean build directory.
CPMAddPackage(
    NAME glfw
    GITHUB_REPOSITORY glfw/glfw
    GIT_TAG 3.4
    SOURCE_DIR "${GFX_RESEARCH_BASE_THIRD_PARTY_DIR}/glfw"
    DOWNLOAD_ONLY YES
)

CPMAddPackage(
    NAME glm
    GITHUB_REPOSITORY g-truc/glm
    GIT_TAG 1.0.1
    SOURCE_DIR "${GFX_RESEARCH_BASE_THIRD_PARTY_DIR}/glm"
    DOWNLOAD_ONLY YES
)

CPMAddPackage(
    NAME imgui
    GITHUB_REPOSITORY ocornut/imgui
    GIT_TAG v1.92.9-docking
    SOURCE_DIR "${GFX_RESEARCH_BASE_THIRD_PARTY_DIR}/imgui"
    DOWNLOAD_ONLY YES
)

CPMAddPackage(
    NAME tinygltf
    GITHUB_REPOSITORY syoyo/tinygltf
    GIT_TAG v2.9.7
    SOURCE_DIR "${GFX_RESEARCH_BASE_THIRD_PARTY_DIR}/tinygltf"
    DOWNLOAD_ONLY YES
)

CPMAddPackage(
    NAME tinyobjloader
    GITHUB_REPOSITORY tinyobjloader/tinyobjloader
    GIT_TAG v2.0-rc1
    SOURCE_DIR "${GFX_RESEARCH_BASE_THIRD_PARTY_DIR}/tinyobjloader"
    DOWNLOAD_ONLY YES
)

CPMAddPackage(
    NAME glad
    GITHUB_REPOSITORY Dav1dde/glad
    GIT_TAG v2.0.8
    SOURCE_DIR "${GFX_RESEARCH_BASE_THIRD_PARTY_DIR}/glad"
    DOWNLOAD_ONLY YES
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory("${GFX_RESEARCH_BASE_THIRD_PARTY_DIR}/glfw" "${CMAKE_CURRENT_BINARY_DIR}/thirdparty/glfw")

add_library(gfx_research_glm INTERFACE)
target_include_directories(gfx_research_glm SYSTEM INTERFACE "${GFX_RESEARCH_BASE_THIRD_PARTY_DIR}/glm")

add_subdirectory("${GFX_RESEARCH_BASE_THIRD_PARTY_DIR}/glad/cmake" "${CMAKE_CURRENT_BINARY_DIR}/thirdparty/glad")
glad_add_library(
    gfx_research_glad
    STATIC
    REPRODUCIBLE
    QUIET
    API gl:core=4.6
    EXTENSIONS NONE
)

add_library(gfx_research_imgui STATIC
    "${GFX_RESEARCH_BASE_THIRD_PARTY_DIR}/imgui/imgui.cpp"
    "${GFX_RESEARCH_BASE_THIRD_PARTY_DIR}/imgui/imgui_draw.cpp"
    "${GFX_RESEARCH_BASE_THIRD_PARTY_DIR}/imgui/imgui_tables.cpp"
    "${GFX_RESEARCH_BASE_THIRD_PARTY_DIR}/imgui/imgui_widgets.cpp"
    "${GFX_RESEARCH_BASE_THIRD_PARTY_DIR}/imgui/backends/imgui_impl_glfw.cpp"
    "${GFX_RESEARCH_BASE_THIRD_PARTY_DIR}/imgui/backends/imgui_impl_opengl3.cpp"
)
target_compile_features(gfx_research_imgui PUBLIC cxx_std_20)
target_include_directories(gfx_research_imgui SYSTEM PUBLIC
    "${GFX_RESEARCH_BASE_THIRD_PARTY_DIR}/imgui"
    "${GFX_RESEARCH_BASE_THIRD_PARTY_DIR}/imgui/backends"
)
target_link_libraries(gfx_research_imgui PUBLIC glfw)
target_compile_definitions(gfx_research_imgui PUBLIC GLFW_INCLUDE_NONE)

add_library(gfx-research-base STATIC
    src/application.cpp
    src/framebuffer.cpp
    src/fullscreen_triangle.cpp
    src/gpu_timer.cpp
    src/mesh.cpp
    src/model.cpp
    src/orbit_camera.cpp
    src/primitives.cpp
    src/readback.cpp
    src/screenshot.cpp
    src/shader.cpp
    src/texture_2d.cpp
)
add_library(gfx::research_base ALIAS gfx-research-base)

target_compile_features(gfx-research-base PUBLIC cxx_std_20)
target_include_directories(gfx-research-base
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)
target_include_directories(gfx-research-base SYSTEM PRIVATE
    "${GFX_RESEARCH_BASE_THIRD_PARTY_DIR}/tinygltf"
    "${GFX_RESEARCH_BASE_THIRD_PARTY_DIR}/tinyobjloader"
)
target_link_libraries(gfx-research-base
    PUBLIC
        glfw
        gfx_research_glad
        gfx_research_glm
        gfx_research_imgui
)
target_compile_definitions(gfx-research-base PUBLIC GLFW_INCLUDE_NONE)

if(GFX_RESEARCH_BASE_BUILD_EXAMPLE)
    add_executable(gfx-research-base-example example/main.cpp)
    target_compile_features(gfx-research-base-example PRIVATE cxx_std_20)
    target_link_libraries(gfx-research-base-example PRIVATE gfx::research_base)
    gfx_research_configure_executable(gfx-research-base-example)
    target_compile_definitions(gfx-research-base-example PRIVATE
        GFX_RESEARCH_BASE_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif()
