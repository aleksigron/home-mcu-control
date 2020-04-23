cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project("Device Server")

# Use C11 standard
set(CMAKE_C_STANDARD 11)

# Don't use language extensions
set(CMAKE_C_EXTENSIONS OFF)

set (EXECUTABLE_NAME device-server)

include_directories(
	src
	include
)

set (DEPS_SOURCES
	deps/civetweb/civetweb.c
)

set (SERVER_SOURCES
	src/main.c
	src/WebServer.c
	src/WebServer.h
)

add_executable(${EXECUTABLE_NAME} ${DEPS_SOURCES} ${SERVER_SOURCES})
