cmake_minimum_required(VERSION 3.23)

if(NOT BUILD_TEST AND BUILD_STATIC)
  set(STATIC_PROXY ON)
endif()

if(BUILD_TEST)
  project(logtest LANGUAGES CXX)
else()
  project(proxy LANGUAGES CXX)
endif()

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

if(NOT STATIC_PROXY)
  add_subdirectory(logclient)
endif()

if (BUILD_TEST)
  add_subdirectory(logtest)
else()
  add_subdirectory(proxy)
  add_subdirectory(monitor)
endif()
