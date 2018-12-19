project( logtest )
set (ProjectDir ${logtest_SOURCE_DIR})
message(STATUS "logtest ProjectDir: " ${ProjectDir})

file( GLOB logtest_SRC *.cpp )
file( GLOB logtest_INC *.h* )

add_executable( logtest ${logtest_SRC} ${logtest_INC})
target_link_libraries(logtest logclient utils ${Boost_LIBRARIES})
target_include_directories(logtest PUBLIC ${ProjectDir}/../logclient ${ProjectDir}/../utils)
install(TARGETS logtest RUNTIME DESTINATION ${DestinationDir})
