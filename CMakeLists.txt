cmake_minimum_required (VERSION 3.15)
project(GLobalProject)

set(CMAKE_BUILD_TYPE Release)

include("build/build/${CMAKE_BUILD_TYPE}/generators/conan_toolchain.cmake")

SET(CMAKE_C_COMPILER /usr/bin/cc)
SET(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3 -march=native ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3 -march=native")



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