file(GLOB INPUT_DEVICE_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(03_InputDevice SHARED ${INPUT_DEVICE_SOURCES})

target_compile_definitions(03_InputDevice PRIVATE MY03INPUTDEVICE_EXPORTS)
target_include_directories(03_InputDevice PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(03_InputDevice
  PUBLIC 04_WindowManager 05_HycUtilitiy
  PRIVATE SDL2::SDL2)
