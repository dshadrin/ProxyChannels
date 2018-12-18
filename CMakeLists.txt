cmake_minimum_required(VERSION 3.11)

set(USE_BOOST_LOGGER OFF)
set(USE_SIMPLE_LOGGER OFF)
set(USE_OSCAR_LOGGER ON)
set(STATIC_PROXY OFF)

project(proxy LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
message(STATUS "Cmake module path: " ${CMAKE_MODULE_PATH}) 

set(APP_BUILD_PATH "${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "Build path: " ${APP_BUILD_PATH})

set(APP_SOURCE_PATH "${CMAKE_SOURCE_DIR}")
message(STATUS "Source path: " ${APP_SOURCE_PATH})

set(DestinationDir ${CMAKE_SOURCE_DIR}/dist)
message(STATUS "DestinationDir: ${DestinationDir}")

include("PlatformSpecific")
include("ProjectBoostLibraries")

include_directories(${APP_SOURCE_PATH})
include_directories(${APP_SOURCE_PATH}/inc)

add_subdirectory(oscar)
add_subdirectory(utils)
add_subdirectory(proxy)
add_subdirectory(logclient)
add_subdirectory(monitor)
add_subdirectory(logtest)
