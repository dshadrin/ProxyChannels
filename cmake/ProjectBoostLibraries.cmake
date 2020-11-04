###
# Boost & platform configuration
###
if(MSVC)
  set(Boost_USE_STATIC_LIBS ON)
#  set(Boost_COMPILER "vc142")
elseif(MINGW)
  set(Boost_USE_STATIC_LIBS ON)
elseif(UNIX)
  set(Boost_USE_STATIC_LIBS ON)
endif()
set(Boost_USE_MULTITHREADED ON)
set(Boost_ARCHITECTURE "-x64")

set(BOOST_LIB_LIST
  system
  filesystem
  thread
  regex
  date_time
  chrono
)

find_package(Boost 1.71 COMPONENTS ${BOOST_LIB_LIST} REQUIRED)
message(STATUS "Boost INCLUDES: ${Boost_INCLUDE_DIRS}")
include_directories(${Boost_INCLUDE_DIRS})
message(STATUS "Boost LIBRARIES: ${Boost_LIBRARIES}")
