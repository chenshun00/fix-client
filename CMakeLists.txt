cmake_minimum_required(VERSION 3.16)

project(fix-client VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)

# 加载quickfix和QT配置包
if(WIN32)
set(CMAKE_PREFIX_PATH "C:/Qt/6.7.2/msvc2019_64" ${CMAKE_PREFIX_PATH})
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake_modules" "C:/Qt/6.7.2/msvc2019_64/lib/cmake" ${CMAKE_MODULE_PATH})
else() 
set(CMAKE_PREFIX_PATH "/Users/chenshun/Qt/6.7.2/macos" ${CMAKE_PREFIX_PATH})
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake_modules" "/Users/chenshun/Qt/6.7.2/macos/lib/cmake" ${CMAKE_MODULE_PATH})
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(TS_FILES fix-client_zh_CN.ts)

if(WIN32)

find_library(
  QUICKFIX_LIBRARY
  NAMES quickfixd
  HINTS "E:/quickfix/lib"
  PATHS "E:/quickfix/lib"
)

else()
find_library(
  QUICKFIX_LIBRARY
  NAMES quickfix
  HINTS /usr/local/lib
  PATHS /usr/local/lib
)
endif()

if(QUICKFIX_LIBRARY)
  message(STATUS "Found QuickFix library: ${QUICKFIX_LIBRARY}")
else()
  message(FATAL_ERROR "Could not find QuickFix library")
endif()


set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(fix-client
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ClientApplication.h
        SessionHolder.h
        SessionHolder.cpp
        ClientApplication.cpp
        Entrust.h
        fixwidget.h fixwidget.cpp fixwidget.ui
        Context.h
        Applicationbridge.h Applicationbridge.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET fix-client APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(fix-client SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(fix-client
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(fix-client PUBLIC Qt${QT_VERSION_MAJOR}::Widgets ${QUICKFIX_LIBRARY})

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.fix-client)
endif()
set_target_properties(fix-client PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS fix-client
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(fix-client)
endif()

if(WIN32)
target_include_directories(fix-client PUBLIC "E:/quickfix/include")
else()
target_include_directories(fix-client PUBLIC /usr/local/include)
endif()
target_include_directories(fix-client INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
