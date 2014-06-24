cmake_minimum_required(VERSION 2.8.9)
# additional cmake files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(lxqt-runner)

# Support Qt5
option(USE_QT5 "Build with Qt5." $ENV{USE_QT5})
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_AUTOMOC ON)
if(USE_QT5)
    cmake_minimum_required(VERSION 2.8.11)
    find_package(Qt5Widgets REQUIRED)
    find_package(Qt5Xml REQUIRED)
    find_package(Qt5Script REQUIRED)
    find_package(Qt5LinguistTools REQUIRED QUIET)
    find_package(lxqt-qt5 REQUIRED)
    find_package(Qt5Xdg REQUIRED)
    find_package(lxqt-globalkeys-qt5 REQUIRED)
    find_package(lxqt-globalkeys-ui-qt5 REQUIRED)
    message(STATUS "Building with Qt${Qt5Core_VERSION_STRING}")
    set(QTX_LIBRARIES Qt5::Widgets Qt5::Xml Qt5::Script)
else()
    find_package(Qt4 REQUIRED QtCore QtGui QtXml QtScript)
    include(${QT_USE_FILE})
    find_package(LXQT REQUIRED)
    find_package(QTXDG REQUIRED)
    find_package(LXQT_GLOBALKEYS REQUIRED)
    find_package(LXQT_GLOBALKEYS_UI REQUIRED)
    message(STATUS "Building with Qt${QTVERSION}")
    set(QTX_LIBRARIES ${QT_LIBRARIES})
endif()
include(${LXQT_USE_FILE})
include(${QTXDG_USE_FILE})
include(${LXQT_GLOBALKEYS_USE_FILE})
include(${LXQT_GLOBALKEYS_UI_USE_FILE})

# optionally use libmenu-cache from lxde to generate the application menu
find_package(PkgConfig)

if (NOT WITHOUT_MENU_CACHE)
    pkg_check_modules(MENU_CACHE
        libmenu-cache>=0.3.3
    )
endif(NOT WITHOUT_MENU_CACHE)

# if libmenu-cache is available
if (MENU_CACHE_FOUND)
    include_directories(${MENU_CACHE_INCLUDE_DIRS})
    add_definitions(-DHAVE_MENU_CACHE=1)
endif (MENU_CACHE_FOUND)

set(lxqt-runner_H_FILES
    dialog.h
    commanditemmodel.h
    widgets.h
    providers.h
    yamlparser.h
    configuredialog/configuredialog.h
)

set(lxqt-runner_CPP_FILES
    main.cpp
    dialog.cpp
    commanditemmodel.cpp
    widgets.cpp
    providers.cpp
    yamlparser.cpp
    configuredialog/configuredialog.cpp
)

set(lxqt-runner_UI_FILES
    dialog.ui
    configuredialog/configuredialog.ui
)

set(QRC_FILES

)

set(lxqt-runner_LIBRARIES
    ${LXQT_LIBRARIES}
    ${LXQT_GLOBALKEYS_LIBRARIES}
    ${LXQT_GLOBALKEYS_UI_LIBRARIES}
    ${QTXDG_LIBRARIES}
    ${MENU_CACHE_LIBRARIES}
    ${QTX_LIBRARIES}
)

macro( setByDefault VAR_NAME VAR_VALUE )
    if (NOT DEFINED ${VAR_NAME})
        set (${VAR_NAME} ${VAR_VALUE})
    endif (NOT DEFINED ${VAR_NAME})
endmacro()


# *******************************************************************
# What plugins will be built, by default.
# You can enable/disable building of the plugin using cmake options.
#    cmake -DRUNNER_MATH=Yes .. # Enable math plugin
#    cmake -DRUNNER_MATH=No ..  # Disable math plugin

setByDefault(RUNNER_MATH Yes )
setByDefault(RUNNER_VBOX Yes )


# Translations **********************************
#file(GLOB TS_FILES
#    ${CMAKE_CURRENT_SOURCE_DIR}/translations/lxqt-runner_*.ts
#)
#qt4_add_translation(QM_FILES ${TS_FILES})
#install(FILES ${QM_FILES} DESTINATION ${LXQT_TRANSLATIONS_DIR})

# FIXME: we should deprecate LxQtTranslate and use .tx scripts later
include(LxQtTranslate)
lxqt_translate_ts(lxqt-runner_QM_FILES 
    SOURCES
        ${lxqt-runner_H_FILES}
        ${lxqt-runner_CPP_FILES} 
        ${lxqt-runner_UI_FILES}
)

#************************************************

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${LXQT_INCLUDE_DIRS}
    ${LXQT_GLOBALKEYS_INCLUDE_DIRS}
    ${LXQT_GLOBALKEYS_UI_INCLUDE_DIRS}
    ${QTXDG_INCLUDE_DIRS}
)

if(NOT CMAKE_BUILD_TYPE)
    set( CMAKE_BUILD_TYPE Release )
endif (NOT CMAKE_BUILD_TYPE)

add_definitions(-Wall)

find_package(X11 REQUIRED)
include_directories(${X11_INCLUDE_DIR})

set(LXQT_SHARE_DIR ${CMAKE_INSTALL_PREFIX}/share/lxqt)
set(APP_SHARE_DIR ${LXQT_SHARE_DIR}/${PROJECT_NAME})

#************************************************

if (RUNNER_MATH)
    add_definitions(-DMATH_ENABLED)
endif (RUNNER_MATH)

# *******************************************************************
# Optional: start VBOX machines in headless mode,
# instead of default window
if (VBOXRUNNER_HEADLESS)
	  add_definitions(-DVBOXRUNNER_HEADLESS)
endif (VBOXRUNNER_HEADLESS)

if (RUNNER_VBOX)
    set(QRC_FILES 
        ${QRC_FILES}
        virtualbox-icons.qrc
    )
    add_definitions(-DVBOX_ENABLED)
endif (RUNNER_VBOX)

if(USE_QT5)
    qt5_wrap_ui(lxqt-runner_UI_HEADERS ${lxqt-runner_UI_FILES})
    qt5_add_resources(lxqt-runner_QRC_SOURCES ${QRC_FILES})
else()
    qt4_wrap_ui(lxqt-runner_UI_HEADERS ${lxqt-runner_UI_FILES})
    qt4_add_resources(lxqt-runner_QRC_SOURCES ${QRC_FILES})
endif()

add_executable(${PROJECT_NAME} 
    ${lxqt-runner_CPP_FILES} 
    ${lxqt-runner_UI_FILES} 
    ${lxqt-runner_QRC_SOURCES} 
    ${lxqt-runner_QM_FILES})

target_link_libraries(${PROJECT_NAME}  ${lxqt-runner_LIBRARIES})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES   ${CONFIG_FILES}    DESTINATION ${APP_SHARE_DIR})

# building tarball with CPack -------------------------------------------------
include (InstallRequiredSystemLibraries)
set (CPACK_PACKAGE_VERSION_MAJOR ${LXQT_MAJOR_VERSION})
set (CPACK_PACKAGE_VERSION_MINOR ${LXQT_MINOR_VERSION})
set (CPACK_PACKAGE_VERSION_PATCH ${LXQT_PATCH_VERSION})
set (CPACK_GENERATOR TBZ2)
set (CPACK_SOURCE_GENERATOR TBZ2)
set (CPACK_SOURCE_IGNORE_FILES /build/;.gitignore;.*~;.git;.kdev4;temp)
include (CPack)
