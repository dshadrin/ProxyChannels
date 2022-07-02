@echo off

set POSTFIX=

if "%1" EQU "" goto cont
set POSTFIX=-%1
:cont

rmdir /S /Q .build%POSTFIX%
mkdir .build%POSTFIX%
cd .build%POSTFIX%
cmake .. -G "Visual Studio 17 2022" -A x64 -Wno-dev -DCMAKE_TOOLCHAIN_FILE=G:/vcpkg/scripts/buildsystems/vcpkg.cmake
cd ../
pause
