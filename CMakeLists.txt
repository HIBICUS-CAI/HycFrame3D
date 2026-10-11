cmake_minimum_required(VERSION 3.20)

project(HycFrame3D LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "build type" FORCE)
endif()

# every binary lands in one folder so shared libraries and the logger
# can be found next to the game executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
set(CMAKE_INSTALL_RPATH "$ORIGIN")

# the code base checks _DEBUG like the visual studio projects did
add_compile_definitions($<$<CONFIG:Debug>:_DEBUG>)
if(MSVC)
  add_compile_definitions(WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS)
endif()

set(HYC_THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/ThirdParty)

find_package(Threads REQUIRED)
find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(OpenAL REQUIRED)
if(NOT WIN32)
  find_package(Bullet REQUIRED)
endif()

add_subdirectory(05_HycUtilitiy)
add_subdirectory(04_WindowManager)
add_subdirectory(03_InputDevice)
add_subdirectory(04_HycLogger)
add_subdirectory(03_RenderSystem_DX11)
add_subdirectory(01_HycFrame3D)
