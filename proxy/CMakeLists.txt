project(proxy LANGUAGES CXX)
set (ProjectDir ${proxy_SOURCE_DIR})
message(STATUS "proxy ProjectDir: " ${ProjectDir})

FILE(GLOB proxy_SRC ${ProjectDir}/*.cpp)
FILE(GLOB proxy_HDR ${ProjectDir}/*.h)

###
# Logger configuration
###
if(USE_OSCAR_LOGGER)
  MESSAGE(STATUS "Defined OSCAR logger")
  list(APPEND proxy_SRC ${ProjectDir}/slogger/oscar_logger/logger.cpp)
  list(APPEND proxy_HDR ${ProjectDir}/slogger/oscar_logger/logger.h)
  list(APPEND proxy_SRC ${ProjectDir}/slogger/oscar_logger/sink.cpp)
  list(APPEND proxy_HDR ${ProjectDir}/slogger/oscar_logger/sink.h)
elseif(USE_BOOST_LOGGER)
  MESSAGE(STATUS "Defined BOOST logger")
  list(APPEND proxy_SRC ${ProjectDir}/slogger/boost_logger/logger.cpp)
  list(APPEND proxy_HDR ${ProjectDir}/slogger/boost_logger/logger.h)
elseif(USE_SIMPLE_LOGGER)
  MESSAGE(STATUS "Defined SIMPLE logger")
else()
  MESSAGE(FATAL_ERROR "No defined any logger")
endif()

###
# App configuration
###
if(STATIC_PROXY)
  add_library(proxy STATIC ${proxy_SRC} ${proxy_HDR})
  target_compile_definitions(proxy PUBLIC STATIC_PROXY)
else()
  add_library(proxy SHARED ${proxy_SRC} ${proxy_HDR})
  target_compile_definitions(proxy PRIVATE PROXY_EXPORTS)
endif()

target_compile_definitions(proxy PRIVATE LOGGING_DIRECT)
target_include_directories(proxy PUBLIC ${ProjectDir})

target_link_libraries(proxy ${Boost_LIBRARIES} oscar utils)

if(USE_OSCAR_LOGGER)
  target_compile_definitions(proxy PUBLIC USE_OSCAR_LOGGER)
  target_include_directories(proxy PUBLIC ${ProjectDir}/slogger/oscar_logger)
elseif(USE_BOOST_LOGGER)
  target_compile_definitions(proxy PUBLIC USE_BOOST_LOGGER)
  target_include_directories(proxy PUBLIC ${ProjectDir}/slogger/boost_logger)
elseif(USE_SIMPLE_LOGGER)
  target_compile_definitions(proxy PUBLIC USE_SIMPLE_LOGGER)
endif()

if(NOT UNIX)
  if(NOT STATIC_PROXY)
    install(TARGETS proxy RUNTIME DESTINATION ${DestinationDir})
  endif()
  if(MINGW)
    target_link_libraries(proxy ws2_32 mswsock)    
  endif()
else()
  target_link_libraries(proxy dl pthread)
  if(STATIC_PROXY)
    target_compile_options(proxy PUBLIC -fPIC)
    target_compile_options(proxy PUBLIC -fpermissive)
  else()
    install(TARGETS proxy LIBRARY DESTINATION ${DestinationDir})
  endif()
endif()

install(FILES ${APP_SOURCE_PATH}/proxy.xml DESTINATION ${DestinationDir})
