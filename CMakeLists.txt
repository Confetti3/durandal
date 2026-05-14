cmake_minimum_required(VERSION 3.20)
project(Durandal VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets WebEngineWidgets Svg)

include(FetchContent)
FetchContent_Declare(
    cmark
    GIT_REPOSITORY https://github.com/commonmark/cmark.git
    GIT_TAG 0.31.1
)

FetchContent_GetProperties(cmark)
if(NOT cmark_POPULATED)
    FetchContent_MakeAvailable(cmark)
endif()

# Generate version header from CMake project version
configure_file(
    ${CMAKE_SOURCE_DIR}/src/Version.h.in
    ${CMAKE_BINARY_DIR}/generated/Version.h
    @ONLY
)

set(SOURCES
    src/main.cpp
    src/VaultManager.cpp
    src/FileTreeModel.cpp
    src/MainWindow.cpp
    src/EditorWidget.cpp
    src/PreviewWidget.cpp
    src/FileTreePanel.cpp
    src/BacklinksPanel.cpp
    src/SearchPanel.cpp
    src/TagPanel.cpp
    src/ToolBar.cpp
    src/StatusBar.cpp
    src/MarkdownRenderer.cpp
    src/WikilinkParser.cpp
    src/SearchEngine.cpp
    src/TagParser.cpp
    src/MarkdownHighlighter.cpp
    src/AutoCompleter.cpp
    src/AppSettings.cpp
)

set(HEADERS
    src/VaultManager.h
    src/FileTreeModel.h
    src/MainWindow.h
    src/EditorWidget.h
    src/PreviewWidget.h
    src/FileTreePanel.h
    src/BacklinksPanel.h
    src/SearchPanel.h
    src/TagPanel.h
    src/ToolBar.h
    src/StatusBar.h
    src/MarkdownRenderer.h
    src/WikilinkParser.h
    src/SearchEngine.h
    src/TagParser.h
    src/MarkdownHighlighter.h
    src/AutoCompleter.h
    src/AppSettings.h
)

set(RESOURCES
    resources/resources.qrc
)

if(WIN32)
    # Copy icon next to generated .rc so the ICON reference resolves
    configure_file(
        ${CMAKE_SOURCE_DIR}/resources/icons/icon.ico
        ${CMAKE_BINARY_DIR}/generated/icon.ico
        COPYONLY
    )
    # Generate Windows .rc with VERSIONINFO from template
    configure_file(
        ${CMAKE_SOURCE_DIR}/resources/windows/Durandal.rc.in
        ${CMAKE_BINARY_DIR}/generated/Durandal.rc
        @ONLY
    )
    set(WIN32_RESOURCES ${CMAKE_BINARY_DIR}/generated/Durandal.rc)
endif()

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
    ${WIN32_RESOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/generated)
target_include_directories(${PROJECT_NAME} PRIVATE ${cmark_SOURCE_DIR}/src)
target_include_directories(${PROJECT_NAME} PRIVATE ${cmark_BINARY_DIR}/src)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::WebEngineWidgets
    Qt6::Svg
    cmark
)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.durandal.app"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE_ICON_FILE "icon.svg"
    )
endif()
