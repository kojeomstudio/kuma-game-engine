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
# Only one .c file needs compiling; headers are exposed to consumers.
add_library(glad STATIC ${CMAKE_SOURCE_DIR}/../third_party/glad/src/gl.c)
target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/../third_party/glad/include)

# ---------------------------------------------------------------------------
# Engine target
# ---------------------------------------------------------------------------

add_executable(kuma-game-engine
  "sources/main.cpp"
  "sources/main.h"
)

target_include_directories(kuma-game-engine PRIVATE sources)
target_link_libraries(kuma-game-engine PRIVATE glfw glad)

# TODO: Add tests and install targets if needed.
