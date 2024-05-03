if(PROJECT_IS_TOP_LEVEL)
  set(
      CMAKE_INSTALL_INCLUDEDIR "include/OMP-SDK-${PROJECT_VERSION}"
      CACHE STRING ""
  )
  set_property(CACHE CMAKE_INSTALL_INCLUDEDIR PROPERTY TYPE PATH)
endif()

# Project is configured with no languages, so tell GNUInstallDirs the lib dir
set(CMAKE_INSTALL_LIBDIR lib CACHE PATH "")

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package OMP-SDK)

install(
    DIRECTORY include/
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT OMP-SDK_Development
)

install(
    TARGETS OMP-SDK_OMP-SDK
    EXPORT OMP-SDKTargets
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
)

# Allow package maintainers to freely override the path for the configs
set(
    OMP-SDK_INSTALL_CMAKEDIR "${CMAKE_INSTALL_DATADIR}/${package}"
    CACHE STRING "CMake package config location relative to the install prefix"
)
set_property(CACHE OMP-SDK_INSTALL_CMAKEDIR PROPERTY TYPE PATH)
mark_as_advanced(OMP-SDK_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${OMP-SDK_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT OMP-SDK_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${OMP-SDK_INSTALL_CMAKEDIR}"
    COMPONENT OMP-SDK_Development
)

install(
    EXPORT OMP-SDKTargets
    NAMESPACE OMP-SDK::
    DESTINATION "${OMP-SDK_INSTALL_CMAKEDIR}"
    COMPONENT OMP-SDK_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
