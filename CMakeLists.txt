cmake_minimum_required(VERSION 3.5)

project(neoscript VERSION 1.0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

option(NEOSCRIPT_BUILD_TESTS      "Build the unit tests when BUILD_TESTING is enabled." ON)
option(NEOSCRIPT_BUILD_STATIC     "Build as static lib."                                ON)
option(NEOSCRIPT_HEADER_ONLY      "Header only build."                                  OFF)
option(NEOSCRIPT_USE_LIBCPP_CLANG "Use libc++ for clang"                                ON)
option(NEOSCRIPT_USE_PRE_GENERATED_PARSERS "Use already generated parsers"              ON)
set   (NEOSCRIPT_BISON            "bison"                                               CACHE STRING "Bison execuatable")
set   (NEOSCRIPT_FLEX             "flex"                                                CACHE STRING "Flex executable"  )

##
## CONFIGURATION
##
set(NEOSCRIPT_TARGET_NAME       ${PROJECT_NAME})
set(NEOSCRIPT_INCLUDE_BUILD_DIR "${PROJECT_SOURCE_DIR}/include")

if (NEOSCRIPT_USE_PRE_GENERATED_PARSERS)

add_custom_command( 

	OUTPUT 
		"${CMAKE_CURRENT_BINARY_DIR}/detail/parse.neo.cpp" 
    "${CMAKE_CURRENT_BINARY_DIR}/detail/lex.neo.cpp" 

   COMMAND ${CMAKE_COMMAND} -E echo "neoscript: Copying parser files - pregenerated parsers"
   COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/detail"
   COMMAND ${CMAKE_COMMAND} -E copy  "${PROJECT_SOURCE_DIR}/gen/detail/parse.neo.cpp" "${PROJECT_SOURCE_DIR}/gen/detail/parse.neo.hpp" "${PROJECT_SOURCE_DIR}/gen/detail/lex.neo.cpp" "${CMAKE_CURRENT_BINARY_DIR}/detail"

 )

else(NEOSCRIPT_USE_PRE_GENERATED_PARSERS)

add_custom_command( 
	
  OUTPUT 
		"${CMAKE_CURRENT_BINARY_DIR}/detail/parse.neo.cpp" 
		
	 COMMAND ${CMAKE_COMMAND} -E echo "neoscript: Building parser"
     COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/detail"
	 COMMAND ${NEOSCRIPT_BISON} -o "${CMAKE_CURRENT_BINARY_DIR}/detail/parse.neo.cpp" --no-lines "${PROJECT_SOURCE_DIR}/src/neo_parser.yy"
   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/detail/parse.neo.cpp" "${CMAKE_CURRENT_BINARY_DIR}/detail/parse.neo.hpp" "${PROJECT_SOURCE_DIR}/gen/detail"

	 DEPENDS 
		"${PROJECT_SOURCE_DIR}/src/neo_parser.yy"
 )
add_custom_command( 

	OUTPUT 
		"${CMAKE_CURRENT_BINARY_DIR}/detail/lex.neo.cpp" 
	
   COMMAND ${CMAKE_COMMAND} -E echo "neoscript: Building lexers"	
   COMMAND ${NEOSCRIPT_FLEX} -o "${CMAKE_CURRENT_BINARY_DIR}/detail/lex.neo.cpp" -L "${PROJECT_SOURCE_DIR}/src/neo_lexer.l"
   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/detail/lex.neo.cpp" "${PROJECT_SOURCE_DIR}/gen/detail"

	 DEPENDS 
		"${PROJECT_SOURCE_DIR}/src/neo_lexer.l"
		"${CMAKE_CURRENT_BINARY_DIR}/detail/parse.neo.cpp"
)

endif(NEOSCRIPT_USE_PRE_GENERATED_PARSERS)

add_custom_target(
  ${NEOSCRIPT_TARGET_NAME}.PreBuild DEPENDS
  ${CMAKE_CURRENT_BINARY_DIR}/detail/parse.neo.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/detail/lex.neo.cpp
)
include(GNUInstallDirs)
##
## TARGET
##
if (NEOSCRIPT_HEADER_ONLY)

  message("nescript is headeronly")

  add_library(${NEOSCRIPT_TARGET_NAME} INTERFACE)
  

  target_include_directories(
    ${NEOSCRIPT_TARGET_NAME}
     INTERFACE 
      $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/src>
      $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/gen>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
  )

  target_compile_options(${NEOSCRIPT_TARGET_NAME} INTERFACE "$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>")
  
else() # LIB

  message("nescript is built as lib")

set(NEOSCRIPT_SOURCES 
    "src/detail/neo_state_machine.cpp"
    "src/detail/neo_command_instance.cpp"
    "src/detail/neo_command_template.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/detail/parse.neo.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/detail/lex.neo.cpp")

if (NEOSCRIPT_BUILD_STATIC)
  add_library(${NEOSCRIPT_TARGET_NAME} STATIC ${NEOSCRIPT_SOURCES})
else()
  add_library(${NEOSCRIPT_TARGET_NAME} SHARED ${NEOSCRIPT_SOURCES})
  target_compile_definitions(${NEOSCRIPT_TARGET_NAME} PUBLIC NEO_DYN_LIB_)
  target_compile_definitions(${NEOSCRIPT_TARGET_NAME} PRIVATE NEO_EXPORT_)
endif()

  target_compile_options(${NEOSCRIPT_TARGET_NAME} PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>")

  target_include_directories(
    ${NEOSCRIPT_TARGET_NAME}
    PUBLIC 
      $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${${PROJECT_NAME}_BINARY_DIR}>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
  )
    
if (NEOSCRIPT_USE_LIBCPP_CLANG)
  target_compile_options(${NEOSCRIPT_TARGET_NAME} PUBLIC "$<$<CXX_COMPILER_ID:Clang>:-stdlib=libc++>")
  target_link_options(${NEOSCRIPT_TARGET_NAME} PUBLIC "$<$<CXX_COMPILER_ID:Clang>:-stdlib=libc++>")
endif(NEOSCRIPT_USE_LIBCPP_CLANG)

endif()

add_dependencies(${NEOSCRIPT_TARGET_NAME} ${NEOSCRIPT_TARGET_NAME}.PreBuild)
add_library(${PROJECT_NAME}::${NEOSCRIPT_TARGET_NAME} ALIAS ${NEOSCRIPT_TARGET_NAME})

##
## TESTS
##
include(CTest) #adds option BUILD_TESTING (default ON)

if(BUILD_TESTING AND NEOSCRIPT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(unit_tests)
    # Utility
    add_subdirectory(utility)
endif()

######################################
# INSTALL
######################################

include(CMakePackageConfigHelpers)

if (NEOSCRIPT_HEADER_ONLY)

install(FILES 
    "${PROJECT_SOURCE_DIR}/gen/detail/parse.neo.cpp"
    "${PROJECT_SOURCE_DIR}/gen/detail/parse.neo.hpp"
    "${PROJECT_SOURCE_DIR}/gen/detail/lex.neo.cpp"
    "${PROJECT_SOURCE_DIR}/src/detail/neo_command_instance.cpp"
    "${PROJECT_SOURCE_DIR}/src/detail/neo_command_template.cpp"
    "${PROJECT_SOURCE_DIR}/src/detail/neo_state_machine.cpp"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/detail/)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}  EXPORT ${PROJECT_NAME}_Targets)

write_basic_package_version_file("${PROJECT_NAME}ConfigVersion.cmake"
                                 VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY SameMajorVersion)
configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION
    cmake)

install(EXPORT ${PROJECT_NAME}_Targets
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION cmake)

install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
              "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION cmake)

else()

install(TARGETS ${PROJECT_NAME}
      EXPORT ${PROJECT_NAME}_Targets
       ARCHIVE DESTINATION lib
       LIBRARY DESTINATION lib
       RUNTIME DESTINATION lib
       )

write_basic_package_version_file("${PROJECT_NAME}ConfigVersion.cmake"
                                 VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY SameMajorVersion)

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION
    lib/cmake)

install(EXPORT ${PROJECT_NAME}_Targets
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION lib/cmake)

install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
              "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION lib/cmake)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

endif()
