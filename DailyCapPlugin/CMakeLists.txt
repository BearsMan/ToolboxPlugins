cmake_minimum_required(VERSION 3.11)

if(NOT(CMAKE_SIZEOF_VOID_P EQUAL 4))
    message(FATAL_ERROR "You are configuring a non 32-bit build, this is not supported. Run cmake with `-A Win32`")
endif()

project(DailyCapPlugin)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${PROJECT_NAME}")

add_compile_options(/MP /permissive- $<$<NOT:$<CONFIG:Debug>>:/MT>)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(gwtoolboxpp)

add_library(DailyCapPlugin SHARED)
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h)
target_include_directories(DailyCapPlugin PRIVATE
        ${gwtoolboxpp_SOURCE_DIR}/plugins/Base
        ${gwtoolboxpp_SOURCE_DIR}/GWToolboxdll)
target_link_libraries(DailyCapPlugin PRIVATE plugin_base gwca)
target_link_options(DailyCapPlugin PRIVATE
        "$<$<CONFIG:Debug>:/NODEFAULTLIB:LIBCMT>"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
target_sources(DailyCapPlugin PRIVATE ${SOURCE_FILES})
target_compile_definitions(DailyCapPlugin PRIVATE
        "NOMINMAX"
        "_WIN32_WINNT=_WIN32_WINNT_WIN7"
        "WIN32_LEAN_AND_MEAN"
        "VC_EXTRALEAN"
        "BUILD_DLL"
)
target_compile_options(DailyCapPlugin PRIVATE /wd4201 /wd4505)
target_compile_options(DailyCapPlugin PRIVATE /W4 /WX /Gy)
target_compile_options(DailyCapPlugin PRIVATE $<$<NOT:$<CONFIG:Debug>>:/GL>)
target_compile_options(DailyCapPlugin PRIVATE $<$<CONFIG:Debug>:/ZI /Od>)
target_link_options(DailyCapPlugin PRIVATE /WX /OPT:REF /OPT:ICF /SAFESEH:NO)
target_link_options(DailyCapPlugin PRIVATE $<$<NOT:$<CONFIG:Debug>>:/LTCG /INCREMENTAL:NO>)
target_link_options(DailyCapPlugin PRIVATE $<$<CONFIG:Debug>:/IGNORE:4098 /OPT:NOREF /OPT:NOICF>)
target_link_options(DailyCapPlugin PRIVATE $<$<CONFIG:RelWithDebInfo>:/OPT:NOICF>)