@echo off

rem set BOOST_INCLUDEDIR=d:\usr\include\boost-1_77
rem set BOOST_LIBRARYDIR=d:\usr\lib
rem set BOOST_INCLUDEDIR=d:\vcpkg\installed\x64-windows-static\include
rem set BOOST_LIBRARYDIR=d:\vcpkg\installed\x64-windows-static\lib

rmdir /S /Q build
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64 -Wno-dev -DVCPKG_TARGET_TRIPLET=x64-windows-static -DBUILD_STATIC:BOOL=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=D:/vcpkg/scripts/buildsystems/vcpkg.cmake
cd ../

rem rmdir /S /Q build-test
rem mkdir build-test
rem cd build-test
rem cmake .. -G "Visual Studio 17 2022" -A x64 -Wno-dev -DBUILD_TEST:BOOL=ON
rem cd ../
pause
