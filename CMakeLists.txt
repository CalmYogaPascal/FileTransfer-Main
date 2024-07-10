cmake_minimum_required (VERSION 3.15)
project(GLobalProject)
include("conan_toolchain.cmake")

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_COMPILER /usr/bin/clang-18)
set(CMAKE_CXX_COMPILER /usr/bin/clang++-18)

if(MSVC)
  add_compile_options(PRIVATE /W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost COMPONENTS program_options regex system REQUIRED)
find_package(Protobuf REQUIRED) 
find_package(gRPC REQUIRED)
find_package(asio-grpc REQUIRED) 


include_directories(PUBLIC ${CMAKE_BINARY_DIR}/Protoses)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(include)


message("Building with CMake version: ${CMAKE_VERSION}")
message("Boost include dirs: ${Boost_INCLUDE_DIRS}")
message("Boost libs: ${Boost_LIBRARIES}")



add_subdirectory(Protoses)
add_subdirectory(Server)
add_subdirectory(Client)