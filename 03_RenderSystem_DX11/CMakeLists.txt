file(GLOB RENDER_SYSTEM_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(03_RenderSystem STATIC ${RENDER_SYSTEM_SOURCES})

target_compile_definitions(03_RenderSystem PUBLIC _RS_VK)
target_include_directories(03_RenderSystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(03_RenderSystem
  PUBLIC 05_HycUtilitiy Vulkan::Vulkan SDL2::SDL2
  PRIVATE glslang::glslang glslang::SPIRV
          glslang::glslang-default-resource-limits Threads::Threads)

# the game loads these files relative to its working directory like the
# pre build step of the visual studio project did
add_custom_target(03_RenderSystem_StaticResources
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_CURRENT_SOURCE_DIR}/StaticResources
          ${CMAKE_SOURCE_DIR}/01_HycFrame3D/RenderSystem_StaticResources
  COMMENT "copying render system static resources")
add_dependencies(03_RenderSystem 03_RenderSystem_StaticResources)
