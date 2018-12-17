#!/bin/sh
rm -fr build-lnx
mkdir build-lnx
cd build-lnx
cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE:STRING="Release"

make install

