project(proxy LANGUAGES CXX)
set (ProjectDir ${proxy_SOURCE_DIR})
message(STATUS "proxy ProjectDir: " ${ProjectDir})

FILE(GLOB proxy_SRC ${ProjectDir}/*.cpp)
FILE(GLOB proxy_HDR ${ProjectDir}/*.h)

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

if(MSVC)
  set_property(TARGET proxy PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

target_compile_definitions(proxy PRIVATE LOGGING_DIRECT)
target_include_directories(proxy PUBLIC ${ProjectDir})

target_link_libraries(proxy ${Boost_LIBRARIES} oscar utils)

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
