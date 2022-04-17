project( monitor )
set (ProjectDir ${monitor_SOURCE_DIR})
message(STATUS "monitor ProjectDir: " ${ProjectDir})

file( GLOB monitor_SRC *.cpp )
file( GLOB monitor_INC *.h* )

if(UNIX)
  if(NOT STATIC_PROXY)
    # use, i.e. don't skip the full RPATH for the build tree
    SET(CMAKE_SKIP_BUILD_RPATH FALSE)

    # when building, don't use the install RPATH already
    # (but later on when installing)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

    # the RPATH to be used when installing, but only if it's not a system directory
    LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${DestinationDir}" isSystemDir)
    IF("${isSystemDir}" STREQUAL "-1")
      SET(CMAKE_INSTALL_RPATH "${DestinationDir}")
    ENDIF("${isSystemDir}" STREQUAL "-1")

    # add the automatically determined parts of the RPATH
    # which point to directories outside the build tree to the install RPATH
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  endif()
endif()

add_executable( monitor ${monitor_SRC} ${monitor_INC})
if(STATIC_PROXY)
  target_compile_definitions(monitor PUBLIC LOGGING_DIRECT)
else()
  target_link_libraries(monitor logclient)
  target_include_directories(monitor PUBLIC ${APP_SOURCE_PATH}/logclient)
endif()

if(MSVC)
  set_property(TARGET monitor PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
target_link_libraries(monitor proxy utils)
target_include_directories(monitor PUBLIC ${APP_SOURCE_PATH}/utils)
install(TARGETS monitor RUNTIME DESTINATION ${DestinationDir})
