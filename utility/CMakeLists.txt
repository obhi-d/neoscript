



## Basic tests
add_executable(validate_script
  src/validate.cpp
  )
target_include_directories(validate_script PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(validate_script PRIVATE ${NEOSCRIPT_TARGET_NAME})
target_compile_features(validate_script PRIVATE cxx_std_17)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" )
  target_compile_options(validate_script PUBLIC "/Zc:__cplusplus")
endif()