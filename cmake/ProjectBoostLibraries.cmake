###
# Boost & platform configuration
###
if(MSVC)
  set(Boost_USE_STATIC_LIBS ON)
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
)

if(USE_BOOST_LOGGER)
  list(APPEND BOOST_LIB_LIST log)
else() #if(USE_SIMPLE_LOGGER)
  list(APPEND BOOST_LIB_LIST regex)
endif()

find_package(Boost COMPONENTS ${BOOST_LIB_LIST} REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
message(STATUS "Boost LIBRARIES: ${Boost_LIBRARIES}")
