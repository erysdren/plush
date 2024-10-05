cmake_minimum_required(VERSION 3.28)
project(Plush
	DESCRIPTION "A neat, portable, realtime 3D rendering library."
	HOMEPAGE_URL "https://github.com/erysdren/plush"
	LANGUAGES C
	VERSION 1.2.0
)

include(CMakePackageConfigHelpers)
include(CMakeDependentOption)
include(CMakePrintHelpers)
include(CheckIncludeFile)
include(GNUInstallDirs)

# environment checks
find_package(SDL2 QUIET)

block(SCOPE_FOR VARIABLES)
  set(CMAKE_REQUIRED_QUIET YES)
  check_include_file(stdint.h HAS_STDINT_H)
endblock()

# options
cmake_dependent_option(PLUSH_BUILD_EXAMPLES "Build Plush Examples" ON
  "TARGET SDL2::SDL2;PROJECT_IS_TOP_LEVEL" OFF)

set(PL_MAX_TRIANGLES 16384 CACHE STRING "Maximum number of triangles per scene")
set(PL_MAX_CHILDREN 16 CACHE STRING "Maximum children per object")
set(PL_MAX_LIGHTS 32 CACHE STRING "Maximum lights per scene")

# sources
configure_file(cmake/pl_conf.h.in include/plush/pl_conf.h @ONLY)
file(GLOB headers CONFIGURE_DEPENDS include/plush/*.h)
file(GLOB sources CONFIGURE_DEPENDS source/*.c)

# library
add_library(plush STATIC)
add_library(plush::plush ALIAS plush)
target_sources(plush PRIVATE ${sources})
target_include_directories(plush
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    # Small hack until all include paths in source/*.c can be fixed
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/plush>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>)
target_link_libraries(plush
  PRIVATE
    $<$<NOT:$<PLATFORM_ID:Windows>>:m>)
set_property(TARGET plush PROPERTY EXPORT_COMPILE_COMMANDS YES)

if (PLUSH_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

# install
write_basic_package_version_file(plush-config-version.cmake
  COMPATIBILITY AnyNewerVersion)
configure_file(cmake/plush.pc.in plush.pc @ONLY)

install(TARGETS plush
  EXPORT plush-targets
  INCLUDES DESTINATION include)
install(EXPORT plush-targets
  DESTINATION lib/cmake
  NAMESPACE plush::
  FILE plush-config.cmake)
install(FILES
  ${PROJECT_BINARY_DIR}/plush-config-version.cmake
  DESTINATION lib/cmake)

install(FILES ${PROJECT_BINARY_DIR}/plush.pc
  DESTINATION lib/pkgconfig)
install(DIRECTORY
  "${PROJECT_SOURCE_DIR}/include/"
  "${PROJECT_BINARY_DIR}/include/"
  TYPE INCLUDE
  FILES_MATCHING
    PATTERN "*.h")
