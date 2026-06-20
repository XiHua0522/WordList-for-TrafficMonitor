cmake_minimum_required(VERSION 3.10)
project(TextReaderPlugin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# 收集源文件
set(SRC_FILES
    TextReader.cpp
    TextReaderItem.cpp
    DataManager.cpp
    Common.cpp
    ChapterParser.cpp
    ChapterDlg.cpp
    BookmarkMgr.cpp
    BookmarkDlg.cpp
    OptionsDlg.cpp
    OptionsContainerDlg.cpp
    TabCtrlEx.cpp
    pch.cpp
)

# 添加 utilities 相关源（简单包含所需文件）
set(UTIL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../utilities)
list(APPEND SRC_FILES
    ${UTIL_DIR}/Common.cpp
    ${UTIL_DIR}/FilePathHelper.cpp
    ${UTIL_DIR}/IniHelper.cpp
    ${UTIL_DIR}/JsonHelper.cpp
    ${UTIL_DIR}/Variant.cpp
    ${UTIL_DIR}/bass64/base64.cpp
)

add_library(TextReader SHARED ${SRC_FILES} TextReader.rc)

# 将输出文件名改为 WordList.dll，保持 target 名称不变以避免改动源码
set_target_properties(TextReader PROPERTIES OUTPUT_NAME "WordList")

target_include_directories(TextReader PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${UTIL_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

if(WIN32)
    target_compile_definitions(TextReader PRIVATE
        _WINDOWS
        _USRDLL
        TextReader_EXPORTS
        UNICODE
        _UNICODE
        _AFXDLL
    )
    target_link_libraries(TextReader PRIVATE
        ole32
        oleaut32
    )
endif()

message(STATUS "TextReader CMake target configured. Output: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
