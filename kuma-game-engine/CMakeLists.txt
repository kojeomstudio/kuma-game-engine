# CMakeList.txt : CMake project for kuma-game-engine, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.20)

# Enable Hot Reload for MSVC compilers if supported.
if(POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(kuma-game-engine VERSION 0.1.0 LANGUAGES C CXX)

# --- C++ standard: require C++20 ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 소스 파일을 UTF-8 로 해석 (한글 주석 C4819 경고 방지).
# MSVC 의 경우 /utf-8 플래그가 소스와 실행 문자집합 모두 UTF-8 로 설정.
if(MSVC)
  add_compile_options(/utf-8)
endif()

# ---------------------------------------------------------------------------
# Third-party dependencies
# ---------------------------------------------------------------------------

# GLFW — included via git submodule at third_party/glfw (forked repo).
# Build it as part of our project with optional parts disabled.
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/../third_party/glfw glfw EXCLUDE_FROM_ALL)

# glad — generated loader sources (GL 3.3 core) at third_party/glad.
# Only one .c file needs compiling; headers are exposed PUBLIC so that
# any consumer that links glad can #include <glad/gl.h>.
add_library(glad STATIC ${CMAKE_SOURCE_DIR}/../third_party/glad/src/gl.c)
target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/../third_party/glad/include)

# nlohmann/json — included via git submodule at third_party/json (forked repo).
# Header-only library; disable its tests/benchmarks to keep build lean.
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")
add_subdirectory(${CMAKE_SOURCE_DIR}/../third_party/json nlohmann_json EXCLUDE_FROM_ALL)

# ---------------------------------------------------------------------------
# Engine target
# ---------------------------------------------------------------------------

add_executable(kuma-game-engine
  # entry
  sources/main.cpp
  sources/main.h
  # core: application + main loop
  sources/core/app/KApplication.cpp
  sources/core/app/KApplication.h
  # platform: window + input (GLFW)
  sources/platform/window/KWindow.cpp
  sources/platform/window/KWindow.h
  # rhi / opengl backend (glad)
  sources/rhi/opengl/KGLContext.cpp
  sources/rhi/opengl/KGLContext.h
  # renderer (high-level)
  sources/renderer/KRenderer.cpp
  sources/renderer/KRenderer.h
  # data: json / config file handling
  sources/data/KJsonWrapper.cpp
  sources/data/KJsonWrapper.h
)

# 헤더 탐색 경로: sources/ 를 루트로 두어 계층형 include 가능
#   예) #include "platform/window/KWindow.h"
#       #include "rhi/opengl/KGLContext.h"
target_include_directories(kuma-game-engine PRIVATE sources)

target_link_libraries(kuma-game-engine PRIVATE glfw glad nlohmann_json::nlohmann_json)

# TODO: Add tests and install targets if needed.
