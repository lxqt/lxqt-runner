cmake_minimum_required(VERSION 2.8.11)

project(lxqt-runner)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_AUTOMOC ON)

option(UPDATE_TRANSLATIONS "Update source translation translations/*.ts files" OFF)

find_package(KF5WindowSystem REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Script REQUIRED)
find_package(Qt5LinguistTools REQUIRED QUIET)
find_package(lxqt REQUIRED)
find_package(Qt5Xdg REQUIRED)
find_package(lxqt-globalkeys REQUIRED)
find_package(lxqt-globalkeys-ui REQUIRED)
message(STATUS "Building with Qt${Qt5Core_VERSION_STRING}")
set(QTX_LIBRARIES Qt5::Widgets Qt5::Xml Qt5::Script)

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
endif()


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



include(LXQtTranslate)

lxqt_translate_ts(lxqt-runner_QM_FILES
    UPDATE_TRANSLATIONS ${UPDATE_TRANSLATIONS}
    SOURCES
        ${lxqt-runner_H_FILES}
        ${lxqt-runner_CPP_FILES}
        ${lxqt-runner_UI_FILES}
    INSTALL_DIR
        "${LXQT_TRANSLATIONS_DIR}/${PROJECT_NAME}"
)

lxqt_app_translation_loader(lxqt-runner_QM_LOADER ${PROJECT_NAME})

#************************************************

include_directories(
    ${LXQT_INCLUDE_DIRS}
    ${LXQT_GLOBALKEYS_INCLUDE_DIRS}
    ${LXQT_GLOBALKEYS_UI_INCLUDE_DIRS}
    ${QTXDG_INCLUDE_DIRS}
)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_definitions(-Wall)

set(LXQT_SHARE_DIR "${CMAKE_INSTALL_PREFIX}/share/lxqt")
set(APP_SHARE_DIR "${LXQT_SHARE_DIR}/${PROJECT_NAME}")

#************************************************

if(NOT DEFINED RUNNER_MATH)
    set(RUNNER_MATH Yes)
endif()

if(RUNNER_MATH)
    add_definitions(-DMATH_ENABLED)
endif(RUNNER_MATH)

# *******************************************************************
# Optional: start VBOX machines in headless mode,
# instead of default window

if(NOT DEFINED RUNNER_VBOX)
    set(RUNNER_VBOX Yes)
endif()

if(VBOXRUNNER_HEADLESS)
    add_definitions(-DVBOXRUNNER_HEADLESS)
endif()

if (RUNNER_VBOX)
    set(QRC_FILES
        ${QRC_FILES}
        virtualbox-icons.qrc
    )
    add_definitions(-DVBOX_ENABLED)
endif()

qt5_wrap_ui(lxqt-runner_UI_HEADERS ${lxqt-runner_UI_FILES})
qt5_add_resources(lxqt-runner_QRC_SOURCES ${QRC_FILES})

add_executable(${PROJECT_NAME}
    ${lxqt-runner_CPP_FILES}
    ${lxqt-runner_UI_FILES}
    ${lxqt-runner_QRC_SOURCES}
    ${lxqt-runner_QM_FILES}
    ${lxqt-runner_QM_LOADER}
)

target_link_libraries(${PROJECT_NAME} ${lxqt-runner_LIBRARIES} KF5::WindowSystem)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES ${CONFIG_FILES} DESTINATION ${APP_SHARE_DIR})

# building tarball with CPack -------------------------------------------------
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_VERSION_MAJOR ${LXQT_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${LXQT_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${LXQT_PATCH_VERSION})
set(CPACK_GENERATOR TBZ2)
set(CPACK_SOURCE_GENERATOR TBZ2)
set(CPACK_SOURCE_IGNORE_FILES /build/;.gitignore;.*~;.git;.kdev4;temp)
include(CPack)
