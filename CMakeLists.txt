cmake_minimum_required(VERSION 3.30)
project(Plush
	DESCRIPTION "A neat, portable, realtime 3D rendering library."
	HOMEPAGE_URL "https://github.com/erysdren/plush"
	LANGUAGES C
	VERSION 1.2.0
)

add_library(plush STATIC
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

target_include_directories(plush PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/source)

include(GNUInstallDirs)
configure_file(${PROJECT_SOURCE_DIR}/cmake/plush.pc.in ${PROJECT_BINARY_DIR}/plush.pc @ONLY)
install(TARGETS plush DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/source/plush.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plush.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
