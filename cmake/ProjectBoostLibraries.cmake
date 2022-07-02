###
# Boost & platform configuration
###
if(MSVC)
  set(Boost_USE_STATIC_LIBS OFF)
  set(Boost_USE_STATIC_RUNTIME OFF)
elseif(MINGW)
  set(Boost_USE_STATIC_LIBS ON)
  add_definitions(-DBOOST_ALLOW_DEPRECATED_HEADERS)
elseif(UNIX)
  add_definitions(-DBOOST_ALLOW_DEPRECATED_HEADERS)
  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_NO_BOOST_CMAKE ON)
endif()
set(Boost_USE_MULTITHREADED ON)
#set(Boost_ARCHITECTURE "-x64")
#set(Boost_DEBUG ON)

set(BOOST_LIB_LIST
  system
  date_time
#  filesystem
  thread
  regex
#  chrono
)

find_package(Boost 1.79 COMPONENTS ${BOOST_LIB_LIST} REQUIRED)
message(STATUS "Boost INCLUDES: ${Boost_INCLUDE_DIRS}")
include_directories(${Boost_INCLUDE_DIRS})
message(STATUS "Boost LIBRARIES: ${Boost_LIBRARIES}")
