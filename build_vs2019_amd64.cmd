@echo off

set BOOST_INCLUDEDIR=d:\usr\include\boost-1_71
set BOOST_LIBRARYDIR=d:\usr\lib

rmdir /S /Q build
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64 -Wno-dev -DBUILD_STATIC:BOOL=ON
cd ../

rmdir /S /Q build-test
mkdir build-test
cd build-test
cmake .. -G "Visual Studio 16 2019" -A x64 -Wno-dev -DBUILD_TEST:BOOL=ON
cd ../
pause
