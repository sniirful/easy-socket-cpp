# CMake minimum version number requirements
cmake_minimum_required(VERSION 3.8)


# project name
set(PROJECT_CC_VERSION_MAJOR 0)
set(PROJECT_CC_VERSION_MINOR 0)
set(PROJECT_CC_VERSION_PATCH 1)
project(easy-socket VERSION ${PROJECT_CC_VERSION_MAJOR}.${PROJECT_CC_VERSION_MINOR}.${PROJECT_CC_VERSION_PATCH})


# output prompt information
message(STATUS "Operation System : ${CMAKE_SYSTEM}")
message(STATUS "CMAKE_INSTALL_PREFIX : ${CMAKE_INSTALL_PREFIX}")
message(STATUS "CMAKE_CXX_COMPILER : ${CMAKE_CXX_COMPILER}")


# configuration file
if (BUILD_SHARED_LIBS STREQUAL "OFF" OR BUILD_STATIC_LIBS STREQUAL "ON")
	set(EASY_SOCKET_BUILD_REPLACE EASY_SOCKET_BUILD_STATIC)
	set(EASY_SERIES_LIBRARY_TYPE "STATIC")
else()
	set(EASY_SOCKET_BUILD_REPLACE EASY_SOCKET_BUILD_SHARED)
	set(EASY_SERIES_LIBRARY_TYPE "SHARED")
endif ()
set(EASY_SOCKET_INCLUDE_REPLACE ${CMAKE_INSTALL_PREFIX}/include)
set(EASY_SOCKET_LIBRARY_REPLACE ${CMAKE_INSTALL_PREFIX}/lib)
configure_file("${CMAKE_SOURCE_DIR}/config/${PROJECT_NAME}-config.cmake.in" "${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake")
configure_file("${CMAKE_SOURCE_DIR}/config/${PROJECT_NAME}.pc.in" "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc")
configure_file("${CMAKE_SOURCE_DIR}/config/VersionInfo.rc.in" "${CMAKE_BINARY_DIR}/VersionInfo.rc")


# Set the module search path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_INSTALL_PREFIX}/lib/cmake)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} $ENV{CXX_DEV}/xanadu/binary/lib/cmake)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_INSTALL_PREFIX}/lib/cmake)
message(STATUS "CMAKE_MODULE_PATH : ${CMAKE_MODULE_PATH}")
message(STATUS "CMAKE_PREFIX_PATH : ${CMAKE_PREFIX_PATH}")


# Configure the build environment
if (MSVC)
	add_compile_options(/W4)
else()
	add_compile_options(-Wall -fpermissive)
endif()
if (EASY_SERIES_LIBRARY_TYPE STREQUAL "STATIC")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -static")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -static")
endif()
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /Od")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD /Od")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG : ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE : ${CMAKE_CXX_FLAGS_RELEASE}")


# set ignore path
set(CMAKE_IGNORE_PATH ${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME})
message(STATUS "CMAKE_IGNORE_PATH : ${CMAKE_IGNORE_PATH}")


# project dependencies
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR})


# C++ support
set(CMAKE_C_STANDARD 17)


# Header file search directory


# link library search directory


# macro definition
if (EASY_SERIES_LIBRARY_TYPE STREQUAL "STATIC")
	add_definitions(-DEASY_SOCKET_BUILD_STATIC)
else()
	add_definitions(-DEASY_SOCKET_BUILD_LOCAL)
endif()
add_definitions(-D_FILE_OFFSET_BITS=64)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)


# Include source files
aux_source_directory(./ SRC)
if (MSVC)
	list(APPEND SRC ${CMAKE_BINARY_DIR}/VersionInfo.rc)
endif ()


# link library
if (WIN32)
	link_libraries("Ws2_32.lib")
elseif (MINGW)
	link_libraries("Ws2_32")
	link_libraries("stdc++")
else ()
	if (${CMAKE_CXX_COMPILER} MATCHES "android")
		link_libraries("android")
	else()
		link_libraries("pthread")
	endif ()
	link_libraries("dl")
endif ()


# Construct
if (EASY_SERIES_LIBRARY_TYPE STREQUAL "STATIC")
	add_library(${PROJECT_NAME} STATIC ${SRC})
else()
	add_library(${PROJECT_NAME} SHARED ${SRC})
endif()


# Set project properties
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_CC_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX -d)


if (EASY_SERIES_LIBRARY_TYPE STREQUAL "STATIC")
	# Set the installation file (static library)
	install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
else()
	# Set the installation file (dynamic library)
	install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
endif()
# Setup installation file (executable)
#install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
# Set the installation file (the PUBLIC header file associated with the library)
#install(TARGETS ${PROJECT_NAME} PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
# Set the installation file (PRIVATE header file associated with the library)
#install(TARGETS ${PROJECT_NAME} PRIVATE_HEADER DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
# Install header files
# install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
install(FILES ${PROJECT_SOURCE_DIR}/sock.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
# Install header files
#install(FILES ${PROJECT_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
# configuration module file
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
