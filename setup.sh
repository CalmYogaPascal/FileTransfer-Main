#!/bin/sh
conan profile detect --force

conan install . --output-folder=build -s build_type=Release --build=missing

cd build

cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .
