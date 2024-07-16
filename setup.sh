#!/bin/sh
echo "Detecting profile"
conan profile detect --force

echo "Installing stuff"
conan install . --output-folder=build --build=missing
conan install . --output-folder=build --build=missing --settings=build_type=Debug

cd build
sh ./build/Release/generators/conanbuild.sh
sh ./build/Debug/generators/conanbuild.sh
#cp ./build/Release ./

cmake .. -DCMAKE_TOOLCHAIN_FILE=./build/Release/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake .. -DCMAKE_TOOLCHAIN_FILE=./build/Debug/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build .