project( logclient )
set (ProjectDir ${logclient_SOURCE_DIR})
message(STATUS "logclient ProjectDir: " ${ProjectDir})

file( GLOB logclient_SRC ${ProjectDir}/*.c* )
file( GLOB logclient_INC ${ProjectDir}/*.h* )

add_library( logclient STATIC ${logclient_SRC} ${logclient_INC} )
target_link_libraries(logclient oscar utils)

if(UNIX)
  target_compile_options(logclient PUBLIC -fPIC)
endif(UNIX)

if(MINGW)
  target_link_libraries(logclient ws2_32)    
endif()

if(MSVC)
  set_property(TARGET logclient PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
