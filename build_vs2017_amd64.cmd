@echo off

set BOOST_INCLUDEDIR=d:\usr\include\boost-1_69
set BOOST_LIBRARYDIR=d:\usr\lib

rmdir /S /Q build
mkdir build
cd build
cmake .. -G "Visual Studio 15 2017 Win64" -DBUILD_STATIC:BOOL=ON
cd ../

rem rmdir /S /Q build-test
rem mkdir build-test
rem cd build-test
rem cmake .. -G "Visual Studio 15 2017 Win64" -DBUILD_TEST:BOOL=ON
rem cd ../
pause
