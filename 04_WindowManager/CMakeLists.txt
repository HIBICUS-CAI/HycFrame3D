file(GLOB WINDOW_MANAGER_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(04_WindowManager SHARED ${WINDOW_MANAGER_SOURCES})

target_compile_definitions(04_WindowManager PRIVATE MY04WINDOWMANAGER_EXPORTS)
target_include_directories(04_WindowManager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(04_WindowManager
  PUBLIC 05_HycUtilitiy
  PRIVATE SDL2::SDL2)
