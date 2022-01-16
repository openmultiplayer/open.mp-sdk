cmake_minimum_required(VERSION 3.19)
project(SDK LANGUAGES C CXX VERSION 0.0.1)

if (NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
	message(
		FATAL_ERROR
		"Clang is the only compiler officially supported for building."
		"Use clang on Unix or clang-cl on Windows to build."
	)
endif()

set(CMAKE_CXX_STANDARD 17)

# MSVC
if(MSVC)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

# Conan third-party libraries
include(lib/cmake-conan/conan.cmake)
include(lib/cmake-conan/conan-omp.cmake)

conan_omp_add_lib(glm 0.9.9.8)
conan_omp_add_lib(abseil 20210324.2)

add_library(SDK INTERFACE)

if(MSVC)
	target_compile_options(SDK INTERFACE
		"/fp:strict"
		"/arch:SSE2"
		"-Werror=format"
	)
elseif(UNIX)
	target_compile_options(SDK INTERFACE -msse2 -mfpmath=sse)
endif()

target_link_libraries(SDK INTERFACE
	CONAN_PKG::glm
	CONAN_PKG::abseil
)

target_compile_definitions(SDK INTERFACE
	GLM_FORCE_SSE2
	GLM_FORCE_QUAT_DATA_WXYZ
	NOMINMAX
)

target_include_directories(SDK INTERFACE include/)

file(GLOB_RECURSE sdk_source_list "*.hpp")

set_property(TARGET SDK PROPERTY SOURCES ${sdk_source_list})
