file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
list(FILTER GAME_SOURCES EXCLUDE REGEX "/Assets/|/RenderSystem_StaticResources/")

add_executable(01_HycFrame3D ${GAME_SOURCES})

target_include_directories(01_HycFrame3D PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/00_BasicFuncs
  ${CMAKE_CURRENT_SOURCE_DIR}/01_MiddleTools
  ${CMAKE_CURRENT_SOURCE_DIR}/02_FrameContent
  ${CMAKE_CURRENT_SOURCE_DIR}/03_ScriptFuncs)

if(WIN32)
  # the vendored bullet headers match the prebuilt windows libraries
  target_include_directories(01_HycFrame3D PRIVATE
    ${HYC_THIRD_PARTY_DIR}/Include/bullet)
  set(HYC_BULLET_LIB_DIR ${HYC_THIRD_PARTY_DIR}/Lib/$<IF:$<CONFIG:Debug>,Deb,Rel>)
  target_link_libraries(01_HycFrame3D PRIVATE
    ${HYC_BULLET_LIB_DIR}/BulletCollision.lib
    ${HYC_BULLET_LIB_DIR}/BulletDynamics.lib
    ${HYC_BULLET_LIB_DIR}/LinearMath.lib)
else()
  target_include_directories(01_HycFrame3D PRIVATE ${BULLET_INCLUDE_DIRS})
  target_link_libraries(01_HycFrame3D PRIVATE ${BULLET_LIBRARIES})
endif()

target_link_libraries(01_HycFrame3D PRIVATE
  05_HycUtilitiy 04_WindowManager 03_InputDevice 03_RenderSystem
  OpenAL::OpenAL Threads::Threads)

# the logger is launched by the game from the same folder
add_dependencies(01_HycFrame3D 04_HycLogger)

# assets are loaded relative to this folder
set_target_properties(01_HycFrame3D PROPERTIES
  VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
