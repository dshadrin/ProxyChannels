@echo off

set path=d:\msys64\mingw64\bin;%PATH%
set BOOST_INCLUDEDIR=d:\usr\include\boost-1_68
set BOOST_LIBRARYDIR=%BOOST_LIBRARYDIR%64

rmdir /S /Q build-mingw
mkdir build-mingw
cd build-mingw
"c:\Program Files\CMake\bin\cmake.exe"  .. -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE:STRING=Release -DBUILD_STATIC:BOOL=ON
cd ../

rmdir /S /Q build-test
mkdir build-test
cd build-test
"c:\Program Files\CMake\bin\cmake.exe"  .. -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE:STRING=Release -DBUILD_TEST:BOOL=ON
cd ../

pause
