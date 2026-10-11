add_executable(04_HycLogger ${CMAKE_CURRENT_SOURCE_DIR}/Main.cpp)

target_link_libraries(04_HycLogger PRIVATE 05_HycUtilitiy Threads::Threads)
