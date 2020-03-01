cmake_minimum_required(VERSION 3.1)

project(neoscript VERSION 0.0.1 LANGUAGES CXX)

option(NEOSCRIPT_BUILD_TESTS "Build the unit tests when BUILD_TESTING is enabled." ON)
option(NEOSCRIPT_USE_LIBCPP_CLANG "Use libc++ for clang" ON)

##
## CONFIGURATION
##
set(NEOSCRIPT_TARGET_NAME               ${PROJECT_NAME})
set(NEOSCRIPT_INCLUDE_BUILD_DIR "${PROJECT_SOURCE_DIR}/include")

add_custom_command( 
	OUTPUT 
		"${CMAKE_CURRENT_BINARY_DIR}/parse.neo.cpp" 
	 COMMAND bison -o ${CMAKE_CURRENT_BINARY_DIR}/parse.neo.cpp --no-lines ${PROJECT_SOURCE_DIR}/include/impl/neo_parser.yy
	 DEPENDS 
		"${PROJECT_SOURCE_DIR}/include/impl/neo_parser.yy"
 )
add_custom_command( 
	OUTPUT 
		"${CMAKE_CURRENT_BINARY_DIR}/lex.neo.cpp" 
	 COMMAND flex -o ${CMAKE_CURRENT_BINARY_DIR}/lex.neo.cpp -L "${PROJECT_SOURCE_DIR}/include/impl/neo_lexer.l"
	 DEPENDS 
		"${PROJECT_SOURCE_DIR}/include/impl/neo_lexer.l"
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


add_library(${NEOSCRIPT_TARGET_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${NEOSCRIPT_TARGET_NAME} ALIAS ${NEOSCRIPT_TARGET_NAME})
add_dependencies(${NEOSCRIPT_TARGET_NAME} ${NEOSCRIPT_TARGET_NAME}.PreBuild)
target_compile_features(${NEOSCRIPT_TARGET_NAME} INTERFACE cxx_std_17)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  if (NEOSCRIPT_USE_LIBCPP_CLANG)
    message("neoscript: Using libc++")
    add_compile_options("-stdlib=libc++")
  endif(NEOSCRIPT_USE_LIBCPP_CLANG)
endif()

target_include_directories(
    ${NEOSCRIPT_TARGET_NAME}
    INTERFACE $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_include_directories(
    ${NEOSCRIPT_TARGET_NAME}
    INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
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
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/parse.neo.cpp ${CMAKE_CURRENT_BINARY_DIR}/lex.neo.cpp DESTINATION ./include/impl)
