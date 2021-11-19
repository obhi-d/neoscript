
  
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
  add_executable(neotest-validity-${test_name} 
    validity/main.cpp
    validity/syntax.cpp
    )
  add_test(validity-${test_name} neotest-validity-${test_name})
  add_dependencies(neotest-validity-${test_name} Catch2-install)
  target_include_directories(neotest-validity-${test_name} PRIVATE "${CMAKE_SOURCE_DIR}/out/external/install/include")
  target_include_directories(neotest-validity-${test_name} PRIVATE "${CMAKE_SOURCE_DIR}/include")
  target_link_libraries(neotest-validity-${test_name} PRIVATE ${NEOSCRIPT_TARGET_NAME})
  target_compile_definitions(neotest-validity-${test_name} PRIVATE ${definitions})
  target_compile_options(neotest-validity-${test_name} PRIVATE ${compile_flags})
  target_compile_features(neotest-validity-${test_name} PRIVATE cxx_std_17)
  target_link_options(neotest-validity-${test_name} PRIVATE ${link_options})
endmacro()
set(NEO_COMMON_CXX_FLAGS "")
set(NEO_COMMON_CXX_LINK_FLAGS "")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if (NEOSCRIPT_USE_LIBCPP_CLANG)
    set(NEO_COMMON_CXX_FLAGS "-fsanitize=address;-fno-omit-frame-pointer;-ftest-coverage;-fprofile-instr-generate;-fcoverage-mapping;-stdlib=libc++")
    set(NEO_COMMON_CXX_LINK_FLAGS "-fsanitize=address;-fprofile-instr-generate;-stdlib=libc++")
  else()
    set(NEO_COMMON_CXX_FLAGS "-fsanitize=address;-fno-omit-frame-pointer;-ftest-coverage;-fprofile-instr-generate;-fcoverage-mapping")
    set(NEO_COMMON_CXX_LINK_FLAGS "-fsanitize=address;-fprofile-instr-generate")
  endif(NEOSCRIPT_USE_LIBCPP_CLANG)
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

file(GLOB NEO_DATASET CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/unit_tests/validity/dataset/*.neoscript)
file(GLOB NEO_EXPECTED CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/unit_tests/validity/expected/*.neoscript)

validity_test("cpp" "" "${NEO_COMMON_CXX_FLAGS}" "${NEO_COMMON_CXX_LINK_FLAGS}")
add_custom_command(TARGET neotest-validity-cpp PRE_BUILD 
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/unit_tests/validity/dataset ${CMAKE_BINARY_DIR}/dataset
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/unit_tests/validity/expected ${CMAKE_BINARY_DIR}/expected
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/output
  DEPENDS ${NEO_DATASET}
  DEPENDS ${NEO_EXPECTED}
)
