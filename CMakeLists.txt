cmake_minimum_required(VERSION 3.24)

project(amd-monitor LANGUAGES CXX)

add_library(amd_monitor_lib INTERFACE)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(AMD_UPROF_ROOT "" CACHE STRING "Root for AMD uProf library")
if ("${AMD_UPROF_ROOT}" STREQUAL "")
    message(FATAL_ERROR "must set AMD_UPROF_ROOT")
endif()
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" "${AMD_UPROF_ROOT}" "${AMD_UPROF_ROOT}/lib/x64")

find_file(AMD_UPROF_CONTROLLER AMDProfileController.h REQUIRED)
get_filename_component(AMD_UPROF_INCLUDE_DIR "${AMD_UPROF_CONTROLLER}" DIRECTORY)

target_include_directories(amd_monitor_lib INTERFACE ${AMD_UPROF_INCLUDE_DIR})

find_library(AMD_PROFILE_CONTROLLER_LIB AMDProfileController REQUIRED)
find_library(AMD_POWER_PROFILE_LIB AMDPowerProfileAPI PATH_SUFFIXES bin REQUIRED)

target_link_libraries(amd_monitor_lib INTERFACE ${AMD_PROFILE_CONTROLLER_LIB} ${AMD_POWER_PROFILE_LIB})

add_executable(amd_monitor main.cpp sql.cpp pwr_counters.cpp)
target_compile_definitions(amd_monitor PRIVATE _GLIBCXX_USE_CXX11_ABI=0)
target_link_libraries(amd_monitor PRIVATE amd_monitor_lib)
target_include_directories(amd_monitor PRIVATE ${CMAKE_CURRENT_LIST_DIR}/duckdb_install)
find_package(fmt REQUIRED)

target_link_libraries(amd_monitor PRIVATE duckdb PRIVATE fmt::fmt-header-only)
target_link_options(amd_monitor PRIVATE "-L${CMAKE_CURRENT_LIST_DIR}/duckdb_install")
set_target_properties(amd_monitor
       PROPERTIES
       LINK_FLAGS "-Wl,-rpath,${CMAKE_CURRENT_LIST_DIR}/duckdb_install"
)