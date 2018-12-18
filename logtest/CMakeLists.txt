project( logtest )
set (ProjectDir ${logtest_SOURCE_DIR})
message(STATUS "logtest ProjectDir: " ${ProjectDir})

file( GLOB logtest_SRC *.cpp )
file( GLOB logtest_INC *.h* )

if(UNIX)
  if(NOT STATIC_COMMUNICATOR)
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

add_executable( logtest ${logtest_SRC} ${logtest_INC})
target_link_libraries(logtest logclient proxy utils)
target_include_directories(logtest PUBLIC ${ProjectDir}/../logclient ${ProjectDir}/../utils)
install(TARGETS logtest RUNTIME DESTINATION ${DestinationDir})
