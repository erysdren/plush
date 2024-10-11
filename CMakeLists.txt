cmake_minimum_required(VERSION 3.28)
project(plush
	DESCRIPTION "A neat, portable, realtime 3D rendering library."
	HOMEPAGE_URL "https://github.com/erysdren/plush"
	LANGUAGES C
	VERSION 1.2.0
)

# options

option(PLUSH_BUILD_EXAMPLES "Build Plush Examples" ON)

# sources

set(PLUSH_SOURCES
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

set(PLUSH_HEADERS
	${PROJECT_SOURCE_DIR}/source/pl_conf.h
	${PROJECT_SOURCE_DIR}/source/pl_defs.h
	${PROJECT_SOURCE_DIR}/source/pl_types.h
	${PROJECT_SOURCE_DIR}/source/plush.h
)

# library

add_library(plush STATIC ${PLUSH_SOURCES})
target_include_directories(plush PUBLIC ${PROJECT_SOURCE_DIR}/source)

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

configure_file(${PROJECT_SOURCE_DIR}/cmake/plush.pc.in ${PROJECT_BINARY_DIR}/plush.pc @ONLY)

install(TARGETS plush DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${PLUSH_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/plush)
install(FILES ${PROJECT_BINARY_DIR}/plush.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

include(CMakePackageConfigHelpers)

configure_package_config_file(
	${PROJECT_SOURCE_DIR}/cmake/Config.cmake.in
	${PROJECT_BINARY_DIR}/PlushConfig.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/plush
	NO_SET_AND_CHECK_MACRO
	NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file(
	"${PROJECT_BINARY_DIR}/PlushConfigVersion.cmake"
	VERSION "${PROJECT_VERSION}"
	COMPATIBILITY AnyNewerVersion
)

install(FILES
	${PROJECT_BINARY_DIR}/PlushConfig.cmake
	${PROJECT_BINARY_DIR}/PlushConfigVersion.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/plush
)
