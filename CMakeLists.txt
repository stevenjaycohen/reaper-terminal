cmake_minimum_required(VERSION 3.20)
project(reaper_terminal VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
option(REAPER_TERMINAL_BUILD_TESTS "Build unit tests" ON)

set(REAPER_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/reaper-sdk" CACHE PATH "REAPER SDK checkout")
set(WDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/WDL/WDL" CACHE PATH "WDL directory")

set(LAUNCHER_SOURCE src/platform/native_terminal_windows.cpp)
if(NOT WIN32)
  set(LAUNCHER_SOURCE src/platform/native_terminal_unix.cpp)
endif()

add_library(native_terminal STATIC src/native_terminal.cpp ${LAUNCHER_SOURCE})
target_include_directories(native_terminal PUBLIC src)

add_library(reaper_terminal MODULE src/reaper_extension.cpp)
target_link_libraries(reaper_terminal PRIVATE native_terminal)
target_include_directories(reaper_terminal PRIVATE ${REAPER_SDK_DIR}/sdk ${WDL_DIR})
if(NOT WIN32)
  target_compile_definitions(reaper_terminal PRIVATE SWELL_PROVIDED_BY_APP)
  target_sources(reaper_terminal PRIVATE
    $<$<PLATFORM_ID:Linux>:${WDL_DIR}/swell/swell-modstub-generic.cpp>
    $<$<PLATFORM_ID:Darwin>:${WDL_DIR}/swell/swell-modstub.mm>)
endif()
set_target_properties(reaper_terminal PROPERTIES PREFIX "" OUTPUT_NAME "reaper_reaper_terminal")
if(APPLE)
  set_target_properties(reaper_terminal PROPERTIES SUFFIX ".dylib")
endif()

if(REAPER_TERMINAL_BUILD_TESTS)
  enable_testing()
  add_executable(terminal_tests tests/native_terminal_tests.cpp)
  target_link_libraries(terminal_tests PRIVATE native_terminal)
  add_test(NAME terminal_tests COMMAND terminal_tests)
endif()
