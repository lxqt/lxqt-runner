cmake_minimum_required(VERSION 2.6)
# additional cmake files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(lxqt-runner)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

find_package(LXQT REQUIRED)
include(${LXQT_USE_FILE})

find_package(QTXDG REQUIRED)
include(${QTXDG_USE_FILE})

find_package(LXQT_GLOBALKEYS REQUIRED)
include(${LXQT_GLOBALKEYS_USE_FILE})

find_package(LXQT_GLOBALKEYS_UI REQUIRED)
include(${LXQT_GLOBALKEYS_UI_USE_FILE})

# optionally use libmenu-cache from lxde to generate the application menu
find_package(PkgConfig)
pkg_check_modules(MENU_CACHE
    libmenu-cache>=0.3.3
)
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

set(lxqt-runner_MOC_FILES
    dialog.h
    commanditemmodel.h
    widgets.h
    configuredialog/configuredialog.h
    providers.h
    yamlparser.h
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
)

set(QT_USE_QTXML true)
set(QT_USE_QTSCRIPT true)


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
find_package(Qt4 REQUIRED QtCore QtGui QtDBus)
include(${QT_USE_FILE})
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

qt4_wrap_cpp(lxqt-runner_MOC_SOURCES ${lxqt-runner_MOC_FILES})
qt4_wrap_ui(lxqt-runner_UI_HEADERS ${lxqt-runner_UI_FILES})
qt4_add_resources(lxqt-runner_QRC_SOURCES ${QRC_FILES})

add_executable(${PROJECT_NAME} 
    ${lxqt-runner_CPP_FILES} 
    ${lxqt-runner_UI_FILES} 
    ${lxqt-runner_QRC_SOURCES} 
    ${lxqt-runner_QM_FILES} 
    ${lxqt-runner_MOC_SOURCES})

target_link_libraries(${PROJECT_NAME}  ${lxqt-runner_LIBRARIES} ${QT_LIBRARIES})

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
