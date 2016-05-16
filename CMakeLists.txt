cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

project(lxqt-runner)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(CMakeDependentOption)
include(GNUInstallDirs)

option(UPDATE_TRANSLATIONS "Update source translation translations/*.ts files" OFF)
option(USE_MENU_CACHE "Use libmenu-cache from LXDE to generate the app menu" ON)
option(RUNNER_MATH "Math operations support" ON)
option(RUNNER_VBOX "Virtual Box support" ON)
cmake_dependent_option(RUNNER_VBOX_HEADLESS
    "Use headless mode instead of default window" OFF
    "RUNNER_VBOX" OFF
)

find_package(KF5WindowSystem REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Script REQUIRED)
find_package(Qt5LinguistTools REQUIRED QUIET)
find_package(lxqt REQUIRED)
find_package(lxqt-globalkeys REQUIRED)
find_package(lxqt-globalkeys-ui REQUIRED)
message(STATUS "Building with Qt${Qt5Core_VERSION_STRING}")

include(LXQtCompilerSettings NO_POLICY_SCOPE)

set(QTX_LIBRARIES Qt5::Widgets Qt5::Xml Qt5::Script)

if (USE_MENU_CACHE)
    # optionally use libmenu-cache from lxde to generate the application menu
    find_package(PkgConfig REQUIRED QUIET)

    pkg_check_modules(MENU_CACHE REQUIRED QUIET
        libmenu-cache>=0.3.3
    )
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
    lxqt
    lxqt-globalkeys
    lxqt-globalkeys-ui
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
    PULL_TRANSLATIONS
        ${PULL_TRANSLATIONS}
    CLEAN_TRANSLATIONS
        ${CLEAN_TRANSLATIONS}
    TRANSLATIONS_REPO
        ${TRANSLATIONS_REPO}
    TRANSLATIONS_REFSPEC
        ${TRANSLATIONS_REFSPEC}
)

lxqt_app_translation_loader(lxqt-runner_QM_LOADER ${PROJECT_NAME})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if (RUNNER_VBOX)
    list(APPEND QRC_FILES virtualbox-icons.qrc)
endif()

add_executable(${PROJECT_NAME}
    ${lxqt-runner_CPP_FILES}
    ${lxqt-runner_QRC_SOURCES}
    ${lxqt-runner_QM_FILES}
    ${lxqt-runner_QM_LOADER}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE "$<$<BOOL:${MENU_CACHE_FOUND}>:${MENU_CACHE_INCLUDE_DIRS}>"
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE "$<$<BOOL:${RUNNER_MATH}>:MATH_ENABLED>"
    PRIVATE "$<$<BOOL:${RUNNER_VBOX}>:VBOX_ENABLED>"
    PRIVATE "$<$<BOOL:${RUNNER_VBOX_HEADLESS}>:VBOX_HEADLESS_ENABLED>"
    PRIVATE "$<$<BOOL:${MENU_CACHE_FOUND}>:HAVE_MENU_CACHE=1>"
)

target_link_libraries(${PROJECT_NAME}
    ${lxqt-runner_LIBRARIES}
    KF5::WindowSystem
)

install(TARGETS
    ${PROJECT_NAME}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    COMPONENT Runtime
)

# building tarball with CPack -------------------------------------------------
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_VERSION_MAJOR ${LXQT_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${LXQT_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${LXQT_PATCH_VERSION})
set(CPACK_GENERATOR TBZ2)
set(CPACK_SOURCE_GENERATOR TBZ2)
set(CPACK_SOURCE_IGNORE_FILES /build/;.gitignore;.*~;.git;.kdev4;temp)
include(CPack)
