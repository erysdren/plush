cmake_minimum_required(VERSION 3.28)
project(plush
	DESCRIPTION "A neat, portable, realtime 3D rendering library."
	HOMEPAGE_URL "https://github.com/erysdren/plush"
	LANGUAGES C
	VERSION 1.2.0
)

# options

option(PLUSH_BUILD_EXAMPLES "Build Plush Examples" ON)
option(PLUSH_BUILD_EDITOR "Build Plush Editor" ON)
option(PLUSH_USE_ASAN "Use Address Sanitizer" OFF)
set(PLUSH_MAX_LIGHTS "32" CACHE STRING "Maximum number of lights in a single render pass")
set(PLUSH_MAX_TRIANGLES "16384" CACHE STRING "Maximum number of triangles in a single render pass")

# asan

add_library(ASAN::ASAN IMPORTED INTERFACE)
target_compile_options(ASAN::ASAN INTERFACE -fsanitize=address $<$<NOT:$<COMPILE_LANG_AND_ID:C,MSVC>>:-fno-omit-frame-pointer>)
target_link_options(ASAN::ASAN INTERFACE -fsanitize=address)

# configure options header

configure_file(${PROJECT_SOURCE_DIR}/cmake/pl_conf.h.in ${PROJECT_SOURCE_DIR}/include/plush/pl_conf.h @ONLY)

# library

add_library(plush STATIC)

add_library(plush::plush ALIAS plush)

# unavoidable with how the rasterizer code is currently structured
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
	target_compile_options(plush PRIVATE -Wno-unsequenced)
endif()

target_sources(plush PRIVATE
	${PROJECT_SOURCE_DIR}/source/cam.c
	${PROJECT_SOURCE_DIR}/source/clip.c
	${PROJECT_SOURCE_DIR}/source/light.c
	${PROJECT_SOURCE_DIR}/source/make.c
	${PROJECT_SOURCE_DIR}/source/mat.c
	${PROJECT_SOURCE_DIR}/source/math.c
	${PROJECT_SOURCE_DIR}/source/mdl.c
	${PROJECT_SOURCE_DIR}/source/obj.c
	${PROJECT_SOURCE_DIR}/source/pf_ptex.c
	${PROJECT_SOURCE_DIR}/source/pf_solid.c
	${PROJECT_SOURCE_DIR}/source/pf_tex.c
	${PROJECT_SOURCE_DIR}/source/pf_trans.c
	${PROJECT_SOURCE_DIR}/source/plush.c
	${PROJECT_SOURCE_DIR}/source/read_3ds.c
	${PROJECT_SOURCE_DIR}/source/read_cob.c
	${PROJECT_SOURCE_DIR}/source/read_jaw.c
	${PROJECT_SOURCE_DIR}/source/read_obj.c
	${PROJECT_SOURCE_DIR}/source/read_pcx.c
	${PROJECT_SOURCE_DIR}/source/readio.c
	${PROJECT_SOURCE_DIR}/source/render.c
	${PROJECT_SOURCE_DIR}/source/spline.c
	${PROJECT_SOURCE_DIR}/source/text.c
	${PROJECT_SOURCE_DIR}/source/texture.c
	${PROJECT_SOURCE_DIR}/source/write_obj.c
	${PROJECT_SOURCE_DIR}/source/write_pcx.c
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

target_link_libraries(plush PUBLIC $<$<BOOL:${PLUSH_USE_ASAN}>:ASAN::ASAN>)

# editor

if(PLUSH_BUILD_EDITOR)
	find_package(SDL3 CONFIG COMPONENTS SDL3)
	if(SDL3_FOUND)
		add_subdirectory(editor)
	else()
		message(WARNING "SDL3 not found, cannot build editor")
	endif()
endif()

# examples

if(PLUSH_BUILD_EXAMPLES)
	set(examples
		wacredits
		duckdemo
		eggbug
		ex1
		ex2
		ex3
		ex4
		fire
		flurry
		fly
		logo
		masked
		metrics
		model
		polyrobo
		polyrobo2
		q3bsp
		reflection
		scene
		ship
		sprite
		teapot
		texenv
		texture
		triangle
	)
	set(files
		# polyrobo.c and polyrobo2.c
		black.pcx
		chrome_grayr.pcx
		chrome_steel1.pcx
		droid_legmap.pcx
		ears.pcx
		eye.pcx
		polyrobo.mtl
		polyrobo.obj

		# polyrobo2.c
		polyrobo.bsp
		susan.obj
		-0lcon0.pcx
		-3lcon0.pcx
		+0~fifties_lgt2.pcx
		cncrt2.pcx
		concrt1.pcx
		concrt1b.pcx
		concrt1c.pcx
		concrt1d.pcx
		measure.pcx
		steel2.pcx

		# reflection.c
		earth.pcx
		moon.pcx

		# ship.c
		citybbk.mtl
		citybbk.obj
		ship.mtl
		ship.obj

		# duckdemo.c
		duckdemo.3ds

		# eggbug.c
		eggbug.3ds
		eggbug.pcx

		# model.c
		fork.3ds

		# fly.c
		ground.pcx
		sky.pcx
		sky2.pcx

		# scene.c
		scene320.col
		scene320.dph
		scene320.pal
		scene640.col
		scene640.dph
		scene640.pal

		# teapot.c and metrics.c 
		teapot.obj

		# ex4.c
		texture1.pcx

		# masked.c
		texture2.pcx

		# fire.c
		brick.pcx
		skull.obj
		skull.pcx
		text01.pcx
		text02.pcx
		text03.pcx
		text04.pcx

		# logo.c
		logo.pcx
		logo.obj
	)
	find_package(SDL2 CONFIG COMPONENTS SDL2)
	find_package(SDL2 CONFIG COMPONENTS SDL2main)
	if(SDL2_FOUND)
		# build sdl2 examples
		foreach(example IN LISTS examples)
			add_executable(${example}-sdl2 WIN32 ${PROJECT_SOURCE_DIR}/examples/${example}.c)
			target_compile_definitions(${example}-sdl2 PUBLIC PLUSH_EXAMPLE_SDL2)
			target_link_libraries(${example}-sdl2 PUBLIC plush SDL2::SDL2 $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>)
			target_link_libraries(${example}-sdl2 PUBLIC $<$<BOOL:${PLUSH_USE_ASAN}>:ASAN::ASAN>)
			if(CMAKE_C_COMPILER_ID STREQUAL "TinyCC")
				target_compile_definitions(${example}-sdl2 PUBLIC SDL_DISABLE_IMMINTRIN_H)
			endif()
			if(EMSCRIPTEN)
				target_compile_options(${example}-sdl3 PUBLIC -sUSE_SDL=2)
				target_link_options(${example}-sdl3 PUBLIC -sUSE_SDL=2)
				target_link_options(${example}-sdl2 PUBLIC --preload-file ${PROJECT_SOURCE_DIR}/examples@/)
			endif()
			if(PSP)
				create_pbp_file(
					TARGET ${example}-sdl2
					ICON_PATH NULL
					BACKGROUND_PATH NULL
					PREVIEW_PATH NULL
					TITLE ${PROJECT_NAME}-${example}-sdl2
					VERSION ${PROJECT_VERSION}
				)
			endif()
		endforeach()
		# copy assets
		foreach(file IN LISTS files)
			configure_file(${PROJECT_SOURCE_DIR}/examples/${file} ${PROJECT_BINARY_DIR}/${file} COPYONLY)
		endforeach()
	else()
		message(WARNING "Cannot build SDL2 Plush examples")
	endif()
	find_package(SDL3 CONFIG COMPONENTS SDL3)
	if(SDL3_FOUND)
		# build sdl3 examples
		foreach(example IN LISTS examples)
			add_executable(${example}-sdl3 WIN32 ${PROJECT_SOURCE_DIR}/examples/${example}.c)
			target_compile_definitions(${example}-sdl3 PUBLIC PLUSH_EXAMPLE_SDL3)
			target_link_libraries(${example}-sdl3 PUBLIC plush SDL3::SDL3)
			target_link_libraries(${example}-sdl3 PUBLIC $<$<BOOL:${PLUSH_USE_ASAN}>:ASAN::ASAN>)
			if(CMAKE_C_COMPILER_ID STREQUAL "TinyCC")
				target_compile_definitions(${example}-sdl3 PUBLIC SDL_DISABLE_IMMINTRIN_H)
			endif()
			if(EMSCRIPTEN)
				target_compile_options(${example}-sdl3 PUBLIC -sUSE_SDL=3)
				target_link_options(${example}-sdl3 PUBLIC -sUSE_SDL=3)
				target_link_options(${example}-sdl3 PUBLIC --preload-file ${PROJECT_SOURCE_DIR}/examples@/)
			endif()
			if(PSP)
				create_pbp_file(
					TARGET ${example}-sdl3
					ICON_PATH NULL
					BACKGROUND_PATH NULL
					PREVIEW_PATH NULL
					TITLE ${PROJECT_NAME}-${example}-sdl3
					VERSION ${PROJECT_VERSION}
				)
			endif()
		endforeach()
		# copy assets
		foreach(file IN LISTS files)
			configure_file(${PROJECT_SOURCE_DIR}/examples/${file} ${PROJECT_BINARY_DIR}/${file} COPYONLY)
		endforeach()
	else()
		message(WARNING "Cannot build SDL3 Plush examples")
	endif()
	if(CMAKE_SYSTEM_NAME STREQUAL "DOS")
		# build dos examples
		foreach(example IN LISTS examples)
			add_executable(${example}-dos ${PROJECT_SOURCE_DIR}/examples/${example}.c)
			target_compile_definitions(${example}-dos PUBLIC PLUSH_EXAMPLE_DOS)
			target_link_libraries(${example}-dos PUBLIC plush)
		endforeach()
		# copy assets
		foreach(file IN LISTS files)
			configure_file(${PROJECT_SOURCE_DIR}/examples/${file} ${PROJECT_BINARY_DIR}/${file} COPYONLY)
		endforeach()
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
