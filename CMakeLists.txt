cmake_minimum_required(VERSION 3.30)
project(Plush LANGUAGES C)

file(GLOB PLUSH_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.h
)

add_library(plush STATIC ${PLUSH_SOURCES})
