# Installation and Packaging Configuration
# CPack configuration for creating installers

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Installation directories
set(INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR})
set(INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
set(INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})
set(INSTALL_DATADIR ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

# CPack configuration
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "InterstellarOutpost Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")

# Optional resource files
if(EXISTS "${CMAKE_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/README.md")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
endif()

# Platform-specific packaging
if(WIN32)
    # Windows NSIS installer
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_NSIS_DISPLAY_NAME "${PROJECT_NAME}")
    set(CPACK_NSIS_PACKAGE_NAME "${PROJECT_NAME}")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/yourusername/${PROJECT_NAME}")
    
    # Create desktop shortcut
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut '$DESKTOP\\\\${PROJECT_NAME}.lnk' '$INSTDIR\\\\bin\\\\${PROJECT_NAME}.exe'"
    )
    set(CPACK_NSIS_DELETE_ICONS_EXTRA
        "Delete '$DESKTOP\\\\${PROJECT_NAME}.lnk'"
    )
elseif(APPLE)
    # macOS bundle
    set(CPACK_GENERATOR "DragNDrop;TGZ")
elseif(UNIX)
    # Linux packages
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
    
    # Debian package specific
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "InterstellarOutpost Team")
    set(CPACK_DEBIAN_PACKAGE_SECTION "games")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    
    # RPM package specific
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
    set(CPACK_RPM_PACKAGE_GROUP "Amusements/Games")
endif()

# Components
set(CPACK_COMPONENTS_ALL Applications Libraries Headers Data)
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "Application")
set(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "Libraries")
set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "C++ Headers")
set(CPACK_COMPONENT_DATA_DISPLAY_NAME "Game Data")

set(CPACK_COMPONENT_APPLICATIONS_DESCRIPTION "InterstellarOutpost main application")
set(CPACK_COMPONENT_LIBRARIES_DESCRIPTION "Runtime libraries")
set(CPACK_COMPONENT_HEADERS_DESCRIPTION "C++ development headers")
set(CPACK_COMPONENT_DATA_DESCRIPTION "Game data files")

# Component dependencies
set(CPACK_COMPONENT_APPLICATIONS_DEPENDS Libraries Data)

include(CPack)
