cmake_minimum_required(VERSION 3.1)

project(neoscript VERSION 0.0.1 LANGUAGES CXX)


option(NEOSCRIPT_BUILD_TESTS "Build the unit tests when BUILD_TESTING is enabled." ON)
option(NEOSCRIPT_BUILD_STATIC "Build as static lib." ON)

##
## CONFIGURATION
##
set(NEOSCRIPT_TARGET_NAME               ${PROJECT_NAME})
set(NEOSCRIPT_INCLUDE_BUILD_DIR "${PROJECT_SOURCE_DIR}/include")

add_custom_command( 
	OUTPUT 
		"${CMAKE_CURRENT_BINARY_DIR}/parse.neo.cpp" 
	 COMMAND bison -o ${CMAKE_CURRENT_BINARY_DIR}/parse.neo.cpp --no-lines ${PROJECT_SOURCE_DIR}/src/neo_parser.yy
	 DEPENDS 
		"${PROJECT_SOURCE_DIR}/src/neo_parser.yy"
 )
add_custom_command( 
	OUTPUT 
		"${CMAKE_CURRENT_BINARY_DIR}/lex.neo.cpp" 
	 COMMAND flex -o ${CMAKE_CURRENT_BINARY_DIR}/lex.neo.cpp -L "${PROJECT_SOURCE_DIR}/src/neo_lexer.l"
	 DEPENDS 
		"${PROJECT_SOURCE_DIR}/src/neo_lexer.l"
		"${CMAKE_CURRENT_BINARY_DIR}/parse.neo.cpp"
    )
add_custom_target(
  ${NEOSCRIPT_TARGET_NAME}.PreBuild DEPENDS
  ${CMAKE_CURRENT_BINARY_DIR}/parse.neo.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/lex.neo.cpp
)
##
## TARGET
##
message("Target name: ${NEOSCRIPT_TARGET_NAME}")
add_library(${NEOSCRIPT_TARGET_NAME} SHARED 
  src/neo_context.cpp
  src/neo_command_instance.cpp
  src/neo_command_template.cpp
  src/neo_interpreter.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/parse.neo.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/lex.neo.cpp
)

target_compile_definitions(${NEOSCRIPT_TARGET_NAME} PRIVATE NEO_EXPORT_)

if (NEOSCRIPT_BUILD_STATIC)
  target_compile_definitions(${NEOSCRIPT_TARGET_NAME} PRIVATE NEO_EXPORT_STATIC_)
endif(NEOSCRIPT_BUILD_STATIC)

add_dependencies(${NEOSCRIPT_TARGET_NAME} ${NEOSCRIPT_TARGET_NAME}.PreBuild)
add_library(${PROJECT_NAME}::${NEOSCRIPT_TARGET_NAME} ALIAS ${NEOSCRIPT_TARGET_NAME})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
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
