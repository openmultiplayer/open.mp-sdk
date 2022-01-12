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

if (NOT CROSS_BUILD)
	if (MSVC_CXX_ARCHITECTURE_ID)
		string(TOLOWER ${MSVC_CXX_ARCHITECTURE_ID} LOWERCASE_CMAKE_SYSTEM_PROCESSOR)
		if (LOWERCASE_CMAKE_SYSTEM_PROCESSOR MATCHES "(x64|x86_64|amd64)")
			set(CROSS_BUILD_ARCH x86_64)
		elseif (LOWERCASE_CMAKE_SYSTEM_PROCESSOR MATCHES "(i[3-6]86|x86)")
			set(CROSS_BUILD_ARCH x86)
		else ()
			message(FATAL_ERROR "Unknown CPU '${LOWERCASE_CMAKE_SYSTEM_PROCESSOR}'")
		endif ()
	else ()
		set(CROSS_BUILD_ARCH x86)
	endif ()
endif()

include(lib/cmake-conan/conan.cmake)

# Automatically download and set up paths to a library on Conan
# Works for multi-configuration
function(add_conan_lib pkg_name conan_pkg)
	set(CONAN_DISABLE_CHECK_COMPILER TRUE)

	conan_check(REQUIRED)

	conan_cmake_run(
		REQUIRES ${conan_pkg}
		ARCH ${CROSS_BUILD_ARCH}
		BASIC_SETUP CMAKE_TARGETS
		BUILD missing
	)

	set_target_properties(${pkg_name} PROPERTIES IMPORTED_GLOBAL TRUE)
endfunction()

add_conan_lib(CONAN_PKG::glm glm/0.9.9.8)
add_conan_lib(CONAN_PKG::abseil abseil/20210324.2)

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
