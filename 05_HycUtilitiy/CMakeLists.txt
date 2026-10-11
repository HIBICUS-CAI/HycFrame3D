# header only utilities and the header only third party libraries
add_library(05_HycUtilitiy INTERFACE)

target_include_directories(05_HycUtilitiy INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${HYC_THIRD_PARTY_DIR}/Include)

if(NOT WIN32)
  # windows sdk ships DirectXMath and sal.h
  target_include_directories(05_HycUtilitiy INTERFACE
    ${HYC_THIRD_PARTY_DIR}/Include/DirectXMath
    ${HYC_THIRD_PARTY_DIR}/Include/sal_stub)
endif()

if(WIN32)
  target_link_libraries(05_HycUtilitiy INTERFACE ws2_32)
endif()
