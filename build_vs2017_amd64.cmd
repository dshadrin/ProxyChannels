@echo off
set BOOST_INCLUDEDIR=d:\usr\include\boost-1_69
set BOOST_LIBRARYDIR=d:\usr\lib
rmdir /S /Q build
mkdir build
cd build
cmake .. -G "Visual Studio 15 2017 Win64"
pause
