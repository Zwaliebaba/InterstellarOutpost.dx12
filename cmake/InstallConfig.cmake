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

# Platform-specific packaging (Windows only)
if(WIN32)
    set(_io_nsis_shortcut_target "${PROJECT_NAME}.exe")
    if(DEFINED APP_CLIENT_OUTPUT_NAME AND NOT APP_CLIENT_OUTPUT_NAME STREQUAL "")
        set(_io_nsis_shortcut_target "${APP_CLIENT_OUTPUT_NAME}.exe")
    endif()
    # Windows NSIS installer
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_NSIS_DISPLAY_NAME "${PROJECT_NAME}")
    set(CPACK_NSIS_PACKAGE_NAME "${PROJECT_NAME}")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/yourusername/${PROJECT_NAME}")

    # Create desktop shortcut
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut '$DESKTOP\\\\${PROJECT_NAME}.lnk' '$INSTDIR\\\\bin\\\\${_io_nsis_shortcut_target}'"
    )
    set(CPACK_NSIS_DELETE_ICONS_EXTRA
        "Delete '$DESKTOP\\\\${PROJECT_NAME}.lnk'"
    )
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
