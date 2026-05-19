cmake_minimum_required(VERSION 3.16)

project(PAINT VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt modules – tambah Svg untuk rendering SVG icon & splash
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Svg LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Svg LinguistTools)

set(TS_FILES PAINT_id_ID.ts)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    scribblearea.cpp
    scribblearea.h
    brushpanel.cpp
    brushpanel.h
    splashscreen.cpp
    splashscreen.h
    resources.qrc
    ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(PAINT
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(PAINT SHARED ${PROJECT_SOURCES})
    else()
        add_executable(PAINT ${PROJECT_SOURCES})
    endif()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

# Link Widgets + Svg
target_link_libraries(PAINT PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
)

# Windows: icon executable dari .ico
# Uncomment baris di bawah HANYA setelah menjalankan generate_icon.py
# sehingga resources/app_icon.ico sudah ada:
# if(WIN32)
#     target_sources(PAINT PRIVATE app_icon.rc)
# endif()

set_target_properties(PAINT PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.ngebukstudio.paint
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS PAINT
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(PAINT)
endif()
