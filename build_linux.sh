#!/bin/sh

rm -fr build-lnx
mkdir build-lnx
cd build-lnx
cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE:STRING="Release" -DBUILD_STATIC:BOOL=ON
cd ../

#rm -fr build-test
#mkdir build-test
#cd build-test
#cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE:STRING="Release" -DBUILD_TEST:BOOL=ON
#cd ../
