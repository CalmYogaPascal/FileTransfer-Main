#!/bin/sh
conan profile detect --force

conan install . --output-folder=build -s build_type=Release --build=missing

cd build
source ./build/Release/generators/conanbuild.sh
#cp ./build/Release ./
cp ./build/Release/generators/conan_toolchain.cmake ../conan_toolchain.cmake

cmake .. -DCMAKE_TOOLCHAIN_FILE=../conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .