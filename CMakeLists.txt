cmake_minimum_required(VERSION 3.1)

project(neoscript VERSION 0.0.1 LANGUAGES CXX)


option(NEOSCRIPT_BUILD_TESTS      "Build the unit tests when BUILD_TESTING is enabled." ON)
option(NEOSCRIPT_BUILD_STATIC     "Build as static lib."                                ON)
option(NEOSCRIPT_HEADER_ONLY      "Header only build."                                  OFF)
option(NEOSCRIPT_USE_LIBCPP_CLANG "Use libc++ for clang"                                ON)
option(NEOSCRIPT_USE_PRE_GENERATED_PARSERS "Use already generated parsers"                       ON)
set   (NEOSCRIPT_BISON            "bison"                                               CACHE STRING "Bison execuatable")
set   (NEOSCRIPT_FLEX              "flex"                                                CACHE STRING "Flex executable"  )

##
## CONFIGURATION
##
set(NEOSCRIPT_TARGET_NAME               ${PROJECT_NAME})
set(NEOSCRIPT_INCLUDE_BUILD_DIR "${PROJECT_SOURCE_DIR}/include")

if (NEOSCRIPT_USE_PRE_GENERATED_PARSERS)

add_custom_command( 
	OUTPUT 
		"${CMAKE_CURRENT_BINARY_DIR}/detail/parse.neo.cpp" 
        "${CMAKE_CURRENT_BINARY_DIR}/detail/lex.neo.cpp" 

   COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/detail"
   COMMAND ${CMAKE_COMMAND} -E copy  "${PROJECT_SOURCE_DIR}/gen/parse.neo.cpp" "${PROJECT_SOURCE_DIR}/gen/parse.neo.hpp" "${PROJECT_SOURCE_DIR}/gen/lex.neo.cpp" "${CMAKE_CURRENT_BINARY_DIR}/detail"
   DEPENDS 
		"${PROJECT_SOURCE_DIR}/src/neo_parser.yy"
        "${PROJECT_SOURCE_DIR}/src/neo_lexer.l"

 )

else(NEOSCRIPT_USE_PRE_GENERATED_PARSERS)

add_custom_command( 
	OUTPUT 
		"${CMAKE_CURRENT_BINARY_DIR}/detail/parse.neo.cpp" 
     COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/detail"
	 COMMAND ${NEOSCRIPT_BISON} -o "${CMAKE_CURRENT_BINARY_DIR}/detail/parse.neo.cpp" --no-lines "${PROJECT_SOURCE_DIR}/src/neo_parser.yy"
     COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/detail/parse.neo.cpp" "${CMAKE_CURRENT_BINARY_DIR}/detail/parse.neo.hpp" "${PROJECT_SOURCE_DIR}/gen"
	 DEPENDS 
		"${PROJECT_SOURCE_DIR}/src/neo_parser.yy"
 )
add_custom_command( 
	OUTPUT 
		"${CMAKE_CURRENT_BINARY_DIR}/detail/lex.neo.cpp" 
	 COMMAND ${NEOSCRIPT_FLEX} -o "${CMAKE_CURRENT_BINARY_DIR}/detail/lex.neo.cpp" -L "${PROJECT_SOURCE_DIR}/src/neo_lexer.l"
     COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/detail/lex.neo.cpp" "${PROJECT_SOURCE_DIR}/gen"
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
##
## TARGET
##
message("Target name: ${NEOSCRIPT_TARGET_NAME}")

if (NEOSCRIPT_HEADER_ONLY)
  add_library(${NEOSCRIPT_TARGET_NAME} INTERFACE)
elseif (NEOSCRIPT_BUILD_STATIC)
  add_library(${NEOSCRIPT_TARGET_NAME} STATIC
    src/neo_state_machine.cpp
    src/neo_command_instance.cpp
    src/neo_command_template.cpp
    "${CMAKE_CURRENT_BINARY_DIR}/detail/parse.neo.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/detail/lex.neo.cpp"
  )
  target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
else()
  add_library(${NEOSCRIPT_TARGET_NAME} SHARED 
    src/neo_state_machine.cpp
    src/neo_command_instance.cpp
    src/neo_command_template.cpp
    "${CMAKE_CURRENT_BINARY_DIR}/detail/parse.neo.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/detail/lex.neo.cpp"
  )

  target_compile_definitions(${NEOSCRIPT_TARGET_NAME} PUBLIC NEO_DYN_LIB_)
  target_compile_definitions(${NEOSCRIPT_TARGET_NAME} PRIVATE NEO_EXPORT_)
  target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
endif()



if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  if (NEOSCRIPT_USE_LIBCPP_CLANG)
    target_compile_options(${NEOSCRIPT_TARGET_NAME} PUBLIC "-stdlib=libc++")
    target_link_options(${NEOSCRIPT_TARGET_NAME} PUBLIC "-stdlib=libc++")
  endif(NEOSCRIPT_USE_LIBCPP_CLANG)
endif()


add_dependencies(${NEOSCRIPT_TARGET_NAME} ${NEOSCRIPT_TARGET_NAME}.PreBuild)
add_library(${PROJECT_NAME}::${NEOSCRIPT_TARGET_NAME} ALIAS ${NEOSCRIPT_TARGET_NAME})

target_include_directories(
    ${NEOSCRIPT_TARGET_NAME}
    PUBLIC $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_include_directories(
    ${NEOSCRIPT_TARGET_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

##
## TESTS
##
include(CTest) #adds option BUILD_TESTING (default ON)

if(BUILD_TESTING AND NEOSCRIPT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(unit_tests)
endif()

# Utility
add_subdirectory(utility)
# INSTALL

install(TARGETS ${PROJECT_NAME}
      EXPORT ${PROJECT_NAME}_Targets
       ARCHIVE DESTINATION lib
       LIBRARY DESTINATION lib
       RUNTIME DESTINATION lib
       )

include(CMakePackageConfigHelpers)
write_basic_package_version_file("${PROJECT_NAME}ConfigVersion.cmake"
                                 VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY SameMajorVersion)

if(DEFINED CMAKE_INSTALL_DATAROOTDIR)
    configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)


    install(EXPORT ${PROJECT_NAME}_Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

    install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)
endif()

install(DIRECTORY include DESTINATION .)

if (NEOSCRIPT_HEADER_ONLY)
install(FILES 
    "${CMAKE_CURRENT_BINARY_DIR}/detail/parse.neo.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/detail/parse.neo.hpp"
    "${CMAKE_CURRENT_BINARY_DIR}/detail/lex.neo.cpp"
    "${PROJECT_SOURCE_DIR}/src/neo_command_instance.cpp"
    "${PROJECT_SOURCE_DIR}/src/neo_command_template.cpp"
    "${PROJECT_SOURCE_DIR}/src/neo_state_machine.cpp"
    DESTINATION include/detail)
endif()