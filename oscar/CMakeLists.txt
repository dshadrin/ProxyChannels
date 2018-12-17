project( oscar )
set (ProjectDir ${oscar_SOURCE_DIR})
message(STATUS "oscar ProjectDir: " ${ProjectDir})

file( GLOB oscar_SRC ${ProjectDir}/*.c* )
file( GLOB oscar_INC ${ProjectDir}/*.h* )

add_library( oscar STATIC ${oscar_SRC} ${oscar_INC} )
if(UNIX)
  target_compile_options(oscar PUBLIC -fPIC)
endif(UNIX)
