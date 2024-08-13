cmake_minimum_required(VERSION 3.16)

project(fix-client VERSION 0.1 LANGUAGES CXX)

set(QT_FORCE_MIN_CMAKE_VERSION_FOR_USING_QT 3.19)
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
        Applicationbridge.h Applicationbridge.cpp
        FixClientLogFactory.h FixClientLogFactory.cpp
        amenddialog.h amenddialog.cpp amenddialog.ui
        canceldialog.h canceldialog.cpp canceldialog.ui
        Resource.qrc
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

if(WIN32)
    target_link_libraries(fix-client PUBLIC Qt${QT_VERSION_MAJOR}::Widgets "${CMAKE_CURRENT_SOURCE_DIR}/lib/quickfix/quickfixd.lib" "${CMAKE_CURRENT_SOURCE_DIR}/lib/spdlog/spdlogd.lib")
else()
    target_link_libraries(fix-client PUBLIC Qt${QT_VERSION_MAJOR}::Widgets "/usr/local/lib/libquickfix.dylib" "/Users/chenshun/CLionProjects/spdlog/build/libspdlog.a")
endif()

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

message("-- GNUInstallDirs: ${GNUInstallDirs}")
message("-- CMAKE_INSTALL_LIBDIR: ${CMAKE_INSTALL_LIBDIR}")

include(GNUInstallDirs)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/quickfix/share/quickfix 
    DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/app/. FILES_MATCHING PATTERN "FIX*.xml"
)

install(TARGETS fix-client
    BUNDLE DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/app/.
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(fix-client)
endif()

target_include_directories(fix-client PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/quickfix/include" "${CMAKE_CURRENT_SOURCE_DIR}/include/spdlog/include")
target_include_directories(fix-client INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

message("-- ${CMAKE_CURRENT_SOURCE_DIR}/include/quickfix/include")
message("-- ${CMAKE_CURRENT_SOURCE_DIR}/include/spdlog/include")
