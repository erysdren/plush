cmake_minimum_required(VERSION 3.30)
project(Plush
	DESCRIPTION "A neat, portable, realtime 3D rendering library."
	HOMEPAGE_URL "https://github.com/erysdren/plush"
	LANGUAGES C
	VERSION 1.2.0
)

# sources

set(PLUSH_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/source/cam.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/clip.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/light.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/make.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/mat.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/math.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/obj.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/pf_ptex.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/pf_solid.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/pf_tex.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/pf_trans.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/plush.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/read_3ds.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/read_cob.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/read_jaw.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/read_pcx.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/render.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/spline.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/text.c
)

set(PLUSH_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/source/pl_conf.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/pl_defs.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/pl_types.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/plush.h
)

# library

add_library(plush STATIC ${PLUSH_SOURCES})
target_include_directories(plush PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/source)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	target_compile_options(plush PRIVATE -Wall -Wextra)
endif()

find_library(MATH m)
if(MATH)
	target_link_libraries(plush PUBLIC ${MATH})
endif()

# examples

find_package(SDL2)

if(SDL2_FOUND)

	add_executable(ex1 ${CMAKE_CURRENT_SOURCE_DIR}/examples/ex1.c)
	target_link_libraries(ex1 PUBLIC plush ${SDL2_LIBRARIES})
	target_include_directories(ex1 PUBLIC ${SDL2_INCLUDE_DIRS})

	add_executable(ex2 ${CMAKE_CURRENT_SOURCE_DIR}/examples/ex2.c)
	target_link_libraries(ex2 PUBLIC plush ${SDL2_LIBRARIES})
	target_include_directories(ex2 PUBLIC ${SDL2_INCLUDE_DIRS})

	add_executable(ex3 ${CMAKE_CURRENT_SOURCE_DIR}/examples/ex3.c)
	target_link_libraries(ex3 PUBLIC plush ${SDL2_LIBRARIES})
	target_include_directories(ex3 PUBLIC ${SDL2_INCLUDE_DIRS})

	add_executable(ex4 ${CMAKE_CURRENT_SOURCE_DIR}/examples/ex4.c)
	target_link_libraries(ex4 PUBLIC plush ${SDL2_LIBRARIES})
	target_include_directories(ex4 PUBLIC ${SDL2_INCLUDE_DIRS})

	# copy texture files

	foreach(file texture1.pcx)
			configure_file(${CMAKE_CURRENT_SOURCE_DIR}/examples/${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} COPYONLY)
	endforeach()

endif()

# install

include(GNUInstallDirs)
configure_file(${PROJECT_SOURCE_DIR}/cmake/plush.pc.in ${PROJECT_BINARY_DIR}/plush.pc @ONLY)
install(TARGETS plush DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${PLUSH_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/plush)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plush.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
