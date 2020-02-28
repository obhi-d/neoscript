
project(cpphash_general_tests)
  
include(ExternalProject)

ExternalProject_Add(Catch2
#  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  SOURCE_DIR "${CMAKE_SOURCE_DIR}/external/Catch2"
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/Catch2
  STEP_TARGETS install 
  CMAKE_ARGS 
  	-DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/out/external/install
		-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
		-DCMAKE_CONFIGURATION_TYPES=${CMAKE_CONFIGURATION_TYPES}
		-DCATCH_BUILD_TESTING=OFF
)



## Basic tests
macro(validity_test test_name definitions compile_flags link_options)
endmacro()
set(NEO_COMMON_CXX_FLAGS "")
set(NEO_COMMON_CXX_LINK_FLAGS "")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(NEO_COMMON_CXX_FLAGS "-fsanitize=address;-fno-omit-frame-pointer;-ftest-coverage;-fprofile-instr-generate;-fcoverage-mapping")
  set(NEO_COMMON_CXX_LINK_FLAGS "-fsanitize=address;-fprofile-instr-generate")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(NEO_COMMON_CXX_FLAGS "-fsanitize=address;-fno-omit-frame-pointer")
  set(NEO_COMMON_CXX_LINK_FLAGS "-fsanitize=address")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  set(NEO_COMMON_CXX_FLAGS "")
  set(NEO_COMMON_CXX_LINK_FLAGS "")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(NEO_COMMON_CXX_FLAGS "")
  set(NEO_COMMON_CXX_LINK_FLAGS "")
endif()

add_executable(neotest-validity 
    validity/main.cpp
    validity/syntax.cpp
    )
  add_test(validity neotest-validity)
  add_dependencies(neotest-validity Catch2-install)
  target_include_directories(neotest-validity PRIVATE "${CMAKE_SOURCE_DIR}/out/external/install/include")
  target_include_directories(neotest-validity PRIVATE "${CMAKE_SOURCE_DIR}/include")
  target_link_libraries(neotest-validity PRIVATE ${NEOSCRIPT_TARGET_NAME})
  target_compile_definitions(neotest-validity PRIVATE ${definitions})
  target_compile_options(neotest-validity PRIVATE ${compile_flags})
  target_compile_features(neotest-validity PRIVATE cxx_std_17)

add_custom_command(TARGET neotest-validity PRE_BUILD 
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/unit_tests/validity/dataset ${CMAKE_BINARY_DIR}/dataset
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/unit_tests/validity/expected ${CMAKE_BINARY_DIR}/expected
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/output)
