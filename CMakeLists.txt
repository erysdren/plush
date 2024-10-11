cmake_minimum_required(VERSION 3.28)
project(plush
	DESCRIPTION "A neat, portable, realtime 3D rendering library."
	HOMEPAGE_URL "https://github.com/erysdren/plush"
	LANGUAGES C
	VERSION 1.2.0
)

# options

option(PLUSH_BUILD_EXAMPLES "Build Plush Examples" ON)

# library

add_library(plush STATIC)

add_library(plush::plush ALIAS plush)

target_sources(plush PRIVATE
	${PROJECT_SOURCE_DIR}/source/cam.c
	${PROJECT_SOURCE_DIR}/source/clip.c
	${PROJECT_SOURCE_DIR}/source/light.c
	${PROJECT_SOURCE_DIR}/source/make.c
	${PROJECT_SOURCE_DIR}/source/mat.c
	${PROJECT_SOURCE_DIR}/source/math.c
	${PROJECT_SOURCE_DIR}/source/obj.c
	${PROJECT_SOURCE_DIR}/source/pf_ptex.c
	${PROJECT_SOURCE_DIR}/source/pf_solid.c
	${PROJECT_SOURCE_DIR}/source/pf_tex.c
	${PROJECT_SOURCE_DIR}/source/pf_trans.c
	${PROJECT_SOURCE_DIR}/source/plush.c
	${PROJECT_SOURCE_DIR}/source/read_3ds.c
	${PROJECT_SOURCE_DIR}/source/read_cob.c
	${PROJECT_SOURCE_DIR}/source/read_jaw.c
	${PROJECT_SOURCE_DIR}/source/read_pcx.c
	${PROJECT_SOURCE_DIR}/source/render.c
	${PROJECT_SOURCE_DIR}/source/spline.c
	${PROJECT_SOURCE_DIR}/source/text.c
	${PROJECT_SOURCE_DIR}/source/texture.c
)

target_include_directories(plush
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
	PRIVATE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>
)

find_library(MATH m)
if(MATH)
	target_link_libraries(plush PUBLIC ${MATH})
endif()

# examples

if(PLUSH_BUILD_EXAMPLES)

	find_package(SDL2)

	if(SDL2_FOUND)

		# build examples

		foreach(example eggbug ex1 ex2 ex3 ex4 fly model texture)
			add_executable(${example} ${PROJECT_SOURCE_DIR}/examples/${example}.c)
			target_link_libraries(${example} PUBLIC plush ${SDL2_LIBRARIES})
			target_include_directories(${example} PUBLIC ${SDL2_INCLUDE_DIRS})
			if(CMAKE_C_COMPILER_ID STREQUAL "TinyCC")
				target_compile_definitions(${example} PUBLIC SDL_DISABLE_IMMINTRIN_H)
			endif()
		endforeach()

		# copy texture files

		foreach(file eggbug.3ds eggbug.pcx fork.3ds ground.pcx sky.pcx sky2.pcx texture1.pcx)
			configure_file(${PROJECT_SOURCE_DIR}/examples/${file} ${PROJECT_BINARY_DIR}/${file} COPYONLY)
		endforeach()

	else()

		message(WARNING "SDL2 not found, cannot build Plush examples")

	endif()

endif()

# install

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

configure_file(${PROJECT_SOURCE_DIR}/cmake/plush.pc.in ${PROJECT_BINARY_DIR}/plush.pc @ONLY)
write_basic_package_version_file(${PROJECT_BINARY_DIR}/plush-config-version.cmake COMPATIBILITY AnyNewerVersion)

install(
	FILES ${PROJECT_BINARY_DIR}/plush.pc
	DESTINATION lib/pkgconfig
)

install(
	TARGETS plush
	EXPORT plush-targets
	INCLUDES DESTINATION include
)

install(
	EXPORT plush-targets
	DESTINATION lib/cmake
	NAMESPACE plush::
	FILE plush-config.cmake
)

install(
	FILES ${PROJECT_BINARY_DIR}/plush-config-version.cmake
	DESTINATION lib/cmake
)

install(
	DIRECTORY
	"${PROJECT_SOURCE_DIR}/include/"
	TYPE INCLUDE
	FILES_MATCHING PATTERN "*.h"
)
