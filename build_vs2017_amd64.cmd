@echo off

set BOOST_INCLUDEDIR=d:\usr\include\boost-1_71
set BOOST_LIBRARYDIR=d:\usr\lib

rmdir /S /Q build
mkdir build
cd build
cmake .. -G "Visual Studio 15 2017 Win64" -DBUILD_STATIC:BOOL=ON
cd ../

rmdir /S /Q build-test
mkdir build-test
cd build-test
cmake .. -G "Visual Studio 15 2017 Win64" -DBUILD_TEST:BOOL=ON
cd ../
pause
