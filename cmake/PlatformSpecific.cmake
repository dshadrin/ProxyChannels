###
# Platform configuration
###
if(MSVC)
  message(STATUS "Toolset: MSVC")
  add_definitions(-DWINVER=0x0600)
  add_definitions(-D_WIN32_WINNT=0x0600)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t /wd4996 /wd4503 /wd4251 /wd4180")
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
elseif(MINGW)
  message(STATUS "Toolset: gcc mingw")
  add_definitions(-DWINVER=0x0502)
  add_definitions(-D_WIN32_WINNT=0x0502)
  add_definitions(-D__STRICT_ANSI__)
  add_definitions(-DMINGW)
elseif(UNIX)
  message(STATUS "Toolset: gcc unix")
endif()
