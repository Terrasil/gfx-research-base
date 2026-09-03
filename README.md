# gfx-research-base

Reusable OpenGL 4.6 foundation for small computer-graphics research projects.

The repository intentionally stays below the level of a game engine. Experiment repositories keep direct access to OpenGL and contain only the code that belongs to a particular research question.

## Requirements

- CMake 3.25 or newer
- a C++20 compiler
- OpenGL 4.6 capable driver
- Git, used by CPM to fetch dependencies on the first configure

## Shared facilities

- GLFW window and OpenGL 4.6 context
- GLAD
- Dear ImGui lifecycle and docking
- GLM
- OBJ and glTF/GLB geometry loading
- GPU mesh and procedural plane, grid, cube and UV sphere
- file-based shaders with reload support
- vertex, tessellation, geometry, fragment and compute stages
- 2D textures
- multi-attachment framebuffers
- fullscreen triangle
- orbit camera
- asynchronous GPU timestamp timer
- CSV output
- PNG framebuffer capture

## Build the example

```bash
cmake -S . -B cmake-build-debug -G Ninja
cmake --build cmake-build-debug --target gfx-research-base-example
```

Drop an `.obj`, `.gltf` or `.glb` file onto the example window, or enter its path in the ImGui panel.

## Use from a research repository

Research repositories import this project through CPM instead of copying the common implementation.

```cmake
cmake_minimum_required(VERSION 3.25)
project(my-research LANGUAGES C CXX)

set(CPM_SOURCE_CACHE "${CMAKE_CURRENT_SOURCE_DIR}/.cpm-cache" CACHE PATH "CPM source cache")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake")

set(GFX_RESEARCH_THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty" CACHE PATH "Third-party source directory")
set(GFX_RESEARCH_BASE_GIT_TAG main CACHE STRING "gfx-research-base Git revision")
set(GFX_RESEARCH_BASE_LOCAL_PATH "" CACHE PATH "Optional local gfx-research-base checkout")

if(GFX_RESEARCH_BASE_LOCAL_PATH)
    add_subdirectory("${GFX_RESEARCH_BASE_LOCAL_PATH}" "${CMAKE_BINARY_DIR}/gfx-research-base")
else()
    CPMAddPackage(
        NAME gfx_research_base
        GITHUB_REPOSITORY Terrasil/gfx-research-base
        GIT_TAG "${GFX_RESEARCH_BASE_GIT_TAG}"
        SOURCE_DIR "${GFX_RESEARCH_THIRD_PARTY_DIR}/gfx-research-base"
        OPTIONS
            "GFX_RESEARCH_BASE_BUILD_EXAMPLE OFF"
    )
endif()

add_executable(my-research src/main.cpp)
target_compile_features(my-research PRIVATE cxx_std_20)
target_link_libraries(my-research PRIVATE gfx::research_base)
```

For local development, configure with for example:

```bash
cmake -S . -B cmake-build-debug -G Ninja \
  -DGFX_RESEARCH_BASE_LOCAL_PATH=../gfx-research-base
```

## Public C++ conventions

The code targets portable C++20. Public code uses the `gfx::research` namespace, `snake_case` functions and methods, trailing underscores for private data members, RAII for OpenGL resources and target-scoped CMake requirements. There are no engine-specific macros, reflection extensions, custom SIMD annotations or global compiler flags.

Types use descriptive `PascalCase` names. OpenGL is intentionally exposed directly so experiments can use features that are not wrapped by the base library.

## Design rule

Infrastructure that is useful to several experiments belongs here. Publication-specific methods, baselines, heuristics, shaders, parameter sweeps and result-processing logic belong in the corresponding `gfx-research-*` repository.

## Dependency notes

The default Dear ImGui dependency uses the `v1.92.9-docking` tag because `AppConfig::docking` enables Dear ImGui docking. If a parent project provides a non-docking Dear ImGui build, the base still compiles and reports that docking is unavailable at runtime.

GLAD is generated reproducibly for OpenGL 4.6 core without optional extensions. tinygltf is used for geometry parsing only; image decoding is handled by `Texture2D`.


## Static Windows executables

`gfx-research-base` and its third-party libraries are built statically. On Windows,
`GFX_RESEARCH_STATIC_RUNTIME` is enabled by default and research executables can call
`gfx_research_configure_executable(<target>)` after `add_executable(...)`. With MinGW
GCC this adds `-static -static-libgcc -static-libstdc++`, avoiding redistribution of
`libstdc++-6.dll`, `libgcc_s_*.dll`, and `libwinpthread-1.dll`. With MSVC the target
uses the static `/MT` runtime. System libraries such as `opengl32.dll`, `user32.dll`,
and `kernel32.dll` remain operating-system dependencies and are not shipped with the
application. Set `-DGFX_RESEARCH_STATIC_RUNTIME=OFF` to opt out.
