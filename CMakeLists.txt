cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    OMP-SDK
    VERSION 0.1.0
    DESCRIPTION "Short description"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES NONE
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(OMP-SDK_OMP-SDK INTERFACE)
add_library(OMP-SDK::OMP-SDK ALIAS OMP-SDK_OMP-SDK)

if(MSVC)
	target_compile_options(OMP-SDK_OMP-SDK INTERFACE
		"/fp:strict"
		"/arch:SSE2"
	)
elseif(UNIX)
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
		target_compile_options(OMP-SDK_OMP-SDK INTERFACE
			-msse2
			-mfpmath=sse
		)
	endif()
endif()

if (NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	target_compile_options(OMP-SDK_OMP-SDK INTERFACE
		-Werror=format
	)
endif()

find_package(glm REQUIRED CONFIG)
find_package(string-view-lite REQUIRED CONFIG)
find_package(robin_hood REQUIRED CONFIG)
find_package(span-lite REQUIRED CONFIG)


target_link_libraries(OMP-SDK_OMP-SDK INTERFACE
	glm
	robin_hood
	span-lite
	string-view-lite
)


target_compile_definitions(OMP-SDK_OMP-SDK INTERFACE
	GLM_FORCE_QUAT_DATA_WXYZ
	NOMINMAX
	nssv_CONFIG_SELECT_STRING_VIEW=nssv_STRING_VIEW_NONSTD
	span_CONFIG_SELECT_SPAN=span_SPAN_NONSTD
)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
	target_compile_definitions(OMP-SDK_OMP-SDK INTERFACE GLM_FORCE_SSE2)
endif()

set_property(
    TARGET OMP-SDK_OMP-SDK PROPERTY
    EXPORT_NAME OMP-SDK
)


target_include_directories(
    OMP-SDK_OMP-SDK ${warning_guard}
    INTERFACE
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_compile_features(OMP-SDK_OMP-SDK INTERFACE cxx_std_17)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT OMP-SDK_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of OMP-SDK"
  )
endif()

include(cmake/dev-mode.cmake)
