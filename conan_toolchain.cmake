# Conan automatically generated toolchain file
# DO NOT EDIT MANUALLY, it will be overwritten

# Avoid including toolchain file several times (bad if appending to variables like
#   CMAKE_CXX_FLAGS. See https://github.com/android/ndk/issues/323
include_guard()
message(STATUS "Using Conan toolchain: ${CMAKE_CURRENT_LIST_FILE}")
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeToolchain' generator only works with CMake >= 3.15")
endif()

########## 'user_toolchain' block #############
# Include one or more CMake user toolchain from tools.cmake.cmaketoolchain:user_toolchain



########## 'generic_system' block #############
# Definition of system, platform and toolset





########## 'compilers' block #############



########## 'arch_flags' block #############
# Define C++ flags, C flags and linker flags from 'settings.arch'

message(STATUS "Conan toolchain: Defining architecture flag: -m64")
string(APPEND CONAN_CXX_FLAGS " -m64")
string(APPEND CONAN_C_FLAGS " -m64")
string(APPEND CONAN_SHARED_LINKER_FLAGS " -m64")
string(APPEND CONAN_EXE_LINKER_FLAGS " -m64")


########## 'libcxx' block #############
# Definition of libcxx from 'compiler.libcxx' setting, defining the
# right CXX_FLAGS for that libcxx

message(STATUS "Conan toolchain: Defining libcxx as C++ flags: -stdlib=libstdc++")
string(APPEND CONAN_CXX_FLAGS " -stdlib=libstdc++")


########## 'cppstd' block #############
# Define the C++ and C standards from 'compiler.cppstd' and 'compiler.cstd'

message(STATUS "Conan toolchain: C++ Standard 17 with extensions ON")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


########## 'extra_flags' block #############
# Include extra C++, C and linker flags from configuration tools.build:<type>flags
# and from CMakeToolchain.extra_<type>_flags

# Conan conf flags start: 
# Conan conf flags end


########## 'cmake_flags_init' block #############
# Define CMAKE_<XXX>_FLAGS from CONAN_<XXX>_FLAGS

foreach(config IN LISTS CMAKE_CONFIGURATION_TYPES)
    string(TOUPPER ${config} config)
    if(DEFINED CONAN_CXX_FLAGS_${config})
      string(APPEND CMAKE_CXX_FLAGS_${config}_INIT " ${CONAN_CXX_FLAGS_${config}}")
    endif()
    if(DEFINED CONAN_C_FLAGS_${config})
      string(APPEND CMAKE_C_FLAGS_${config}_INIT " ${CONAN_C_FLAGS_${config}}")
    endif()
    if(DEFINED CONAN_SHARED_LINKER_FLAGS_${config})
      string(APPEND CMAKE_SHARED_LINKER_FLAGS_${config}_INIT " ${CONAN_SHARED_LINKER_FLAGS_${config}}")
    endif()
    if(DEFINED CONAN_EXE_LINKER_FLAGS_${config})
      string(APPEND CMAKE_EXE_LINKER_FLAGS_${config}_INIT " ${CONAN_EXE_LINKER_FLAGS_${config}}")
    endif()
endforeach()

if(DEFINED CONAN_CXX_FLAGS)
  string(APPEND CMAKE_CXX_FLAGS_INIT " ${CONAN_CXX_FLAGS}")
endif()
if(DEFINED CONAN_C_FLAGS)
  string(APPEND CMAKE_C_FLAGS_INIT " ${CONAN_C_FLAGS}")
endif()
if(DEFINED CONAN_SHARED_LINKER_FLAGS)
  string(APPEND CMAKE_SHARED_LINKER_FLAGS_INIT " ${CONAN_SHARED_LINKER_FLAGS}")
endif()
if(DEFINED CONAN_EXE_LINKER_FLAGS)
  string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT " ${CONAN_EXE_LINKER_FLAGS}")
endif()


########## 'extra_variables' block #############
# Definition of extra CMake variables from tools.cmake.cmaketoolchain:extra_variables



########## 'try_compile' block #############
# Blocks after this one will not be added when running CMake try/checks

get_property( _CMAKE_IN_TRY_COMPILE GLOBAL PROPERTY IN_TRY_COMPILE )
if(_CMAKE_IN_TRY_COMPILE)
    message(STATUS "Running toolchain IN_TRY_COMPILE")
    return()
endif()


########## 'find_paths' block #############
# Define paths to find packages, programs, libraries, etc.

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

# Definition of CMAKE_MODULE_PATH
list(PREPEND CMAKE_MODULE_PATH "/home/mark2/.conan2/p/b/openscf1f05b54db92/p/lib/cmake" "/home/mark2/.conan2/p/b/proto03e38f1501ff7/p/lib/cmake/protobuf")
# the generators folder (where conan generates files, like this toolchain)
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

# Definition of CMAKE_PREFIX_PATH, CMAKE_XXXXX_PATH
# The explicitly defined "builddirs" of "host" context dependencies must be in PREFIX_PATH
list(PREPEND CMAKE_PREFIX_PATH "/home/mark2/.conan2/p/b/openscf1f05b54db92/p/lib/cmake" "/home/mark2/.conan2/p/b/proto03e38f1501ff7/p/lib/cmake/protobuf")
# The Conan local "generators" folder, where this toolchain is saved.
list(PREPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_LIST_DIR} )
list(PREPEND CMAKE_LIBRARY_PATH "/home/mark2/.conan2/p/b/cryptf232a025d325e/p/lib" "/home/mark2/.conan2/p/b/boost4fa7aa42ead51/p/lib" "/home/mark2/.conan2/p/b/libbafbf7ddcec30af/p/lib" "/home/mark2/.conan2/p/b/grpc48687b4332a3b/p/lib" "/home/mark2/.conan2/p/b/absei9fd8c6e7917d2/p/lib" "/home/mark2/.conan2/p/b/c-are71bc54c0d225e/p/lib" "/home/mark2/.conan2/p/b/openscf1f05b54db92/p/lib" "/home/mark2/.conan2/p/b/re2d949cf344d442/p/lib" "/home/mark2/.conan2/p/b/libsy39c786102cf50/p/lib" "/home/mark2/.conan2/p/b/libcaf5d47b0b04ea0/p/lib" "/home/mark2/.conan2/p/b/libmo5a65a56782508/p/lib" "/home/mark2/.conan2/p/b/libxcecac7fa25a5c1/p/lib" "/home/mark2/.conan2/p/b/libsedbcb46d4a6723/p/lib" "/home/mark2/.conan2/p/b/pcre2bc89f77244c62/p/lib" "/home/mark2/.conan2/p/b/bzip21d4919e4ffb9d/p/lib" "/home/mark2/.conan2/p/b/lz45ee23d4f54d69/p/lib" "/home/mark2/.conan2/p/b/xz_utd5a3710f0cb94/p/lib" "/home/mark2/.conan2/p/b/zstdd9a636da6b507/p/lib" "/home/mark2/.conan2/p/b/proto03e38f1501ff7/p/lib" "/home/mark2/.conan2/p/b/zlib5c151a22ba0d5/p/lib")
list(PREPEND CMAKE_INCLUDE_PATH "/home/mark2/.conan2/p/b/cryptf232a025d325e/p/include" "/home/mark2/.conan2/p/asio-f30bb22ddc689/p/include" "/home/mark2/.conan2/p/b/boost4fa7aa42ead51/p/include" "/home/mark2/.conan2/p/b/libbafbf7ddcec30af/p/include" "/home/mark2/.conan2/p/b/grpc48687b4332a3b/p/include" "/home/mark2/.conan2/p/b/absei9fd8c6e7917d2/p/include" "/home/mark2/.conan2/p/b/c-are71bc54c0d225e/p/include" "/home/mark2/.conan2/p/b/openscf1f05b54db92/p/include" "/home/mark2/.conan2/p/b/re2d949cf344d442/p/include" "/home/mark2/.conan2/p/b/libsy39c786102cf50/p/include" "/home/mark2/.conan2/p/b/libcaf5d47b0b04ea0/p/include" "/home/mark2/.conan2/p/b/libmo5a65a56782508/p/include" "/home/mark2/.conan2/p/b/libmo5a65a56782508/p/include/libmount" "/home/mark2/.conan2/p/b/libxcecac7fa25a5c1/p/include" "/home/mark2/.conan2/p/b/libsedbcb46d4a6723/p/include" "/home/mark2/.conan2/p/b/pcre2bc89f77244c62/p/include" "/home/mark2/.conan2/p/b/bzip21d4919e4ffb9d/p/include" "/home/mark2/.conan2/p/b/lz45ee23d4f54d69/p/include" "/home/mark2/.conan2/p/b/xz_utd5a3710f0cb94/p/include" "/home/mark2/.conan2/p/b/zstdd9a636da6b507/p/include" "/home/mark2/.conan2/p/b/proto03e38f1501ff7/p/include" "/home/mark2/.conan2/p/b/zlib5c151a22ba0d5/p/include")
set(CONAN_RUNTIME_LIB_DIRS "/home/mark2/.conan2/p/b/cryptf232a025d325e/p/lib" "/home/mark2/.conan2/p/b/boost4fa7aa42ead51/p/lib" "/home/mark2/.conan2/p/b/libbafbf7ddcec30af/p/lib" "/home/mark2/.conan2/p/b/grpc48687b4332a3b/p/lib" "/home/mark2/.conan2/p/b/absei9fd8c6e7917d2/p/lib" "/home/mark2/.conan2/p/b/c-are71bc54c0d225e/p/lib" "/home/mark2/.conan2/p/b/openscf1f05b54db92/p/lib" "/home/mark2/.conan2/p/b/re2d949cf344d442/p/lib" "/home/mark2/.conan2/p/b/libsy39c786102cf50/p/lib" "/home/mark2/.conan2/p/b/libcaf5d47b0b04ea0/p/lib" "/home/mark2/.conan2/p/b/libmo5a65a56782508/p/lib" "/home/mark2/.conan2/p/b/libxcecac7fa25a5c1/p/lib" "/home/mark2/.conan2/p/b/libsedbcb46d4a6723/p/lib" "/home/mark2/.conan2/p/b/pcre2bc89f77244c62/p/lib" "/home/mark2/.conan2/p/b/bzip21d4919e4ffb9d/p/lib" "/home/mark2/.conan2/p/b/lz45ee23d4f54d69/p/lib" "/home/mark2/.conan2/p/b/xz_utd5a3710f0cb94/p/lib" "/home/mark2/.conan2/p/b/zstdd9a636da6b507/p/lib" "/home/mark2/.conan2/p/b/proto03e38f1501ff7/p/lib" "/home/mark2/.conan2/p/b/zlib5c151a22ba0d5/p/lib" )



########## 'pkg_config' block #############
# Define pkg-config from 'tools.gnu:pkg_config' executable and paths

if (DEFINED ENV{PKG_CONFIG_PATH})
set(ENV{PKG_CONFIG_PATH} "${CMAKE_CURRENT_LIST_DIR}:$ENV{PKG_CONFIG_PATH}")
else()
set(ENV{PKG_CONFIG_PATH} "${CMAKE_CURRENT_LIST_DIR}:")
endif()


########## 'rpath' block #############
# Defining CMAKE_SKIP_RPATH



########## 'output_dirs' block #############
# Definition of CMAKE_INSTALL_XXX folders

set(CMAKE_INSTALL_BINDIR "bin")
set(CMAKE_INSTALL_SBINDIR "bin")
set(CMAKE_INSTALL_LIBEXECDIR "bin")
set(CMAKE_INSTALL_LIBDIR "lib")
set(CMAKE_INSTALL_INCLUDEDIR "include")
set(CMAKE_INSTALL_OLDINCLUDEDIR "include")


########## 'variables' block #############
# Definition of CMake variables from CMakeToolchain.variables values

# Variables
# Variables  per configuration



########## 'preprocessor' block #############
# Preprocessor definitions from CMakeToolchain.preprocessor_definitions values

# Preprocessor definitions per configuration



if(CMAKE_POLICY_DEFAULT_CMP0091)  # Avoid unused and not-initialized warnings
endif()
