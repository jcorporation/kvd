# SPDX-License-Identifier: GPL-3.0-or-later
# (c) 2026 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/kvd

cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
cmake_policy(SET CMP0003 NEW)

# KVD is written in C
# supported compilers: gcc, clang
project(kvd
  VERSION 0.0.1
  LANGUAGES C
)

# output binaries in bin directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

message("Cmake version: ${CMAKE_VERSION}")
message("Cmake src dir: ${PROJECT_SOURCE_DIR}")
message("Cmake build dir: ${CMAKE_CURRENT_BINARY_DIR}")
message("Cmake build type: ${CMAKE_BUILD_TYPE}")
message("Cmake generator: ${CMAKE_GENERATOR}")
message("Compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message("CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
message("CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")
message("Arch: ${CMAKE_C_LIBRARY_ARCHITECTURE}")

# reset cmake default flags
set(CMAKE_C_FLAGS_DEBUG "")
set(CMAKE_C_FLAGS_RELEASE "")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "")
set(CMAKE_C_FLAGS_MINSIZEREL "")

# cmake modules
include(CheckCCompilerFlag)
include(CheckCSourceCompiles)
include(CheckIPOSupported)
include(GNUInstallDirs)

# custom cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

# calculate paths
if(CMAKE_INSTALL_PREFIX MATCHES "/usr")
  set(SUBDIR "/${PROJECT_NAME}")
  set(SUBDIRLIB "/lib")
else()
  # for install in /opt
  set(SUBDIR "")
  set(SUBDIRLIB "")
  set(SUBDIRCACHE "")
endif()

message("Executables in: ${CMAKE_INSTALL_FULL_BINDIR}")

find_library(MATH_LIB m REQUIRED)

# configure some files - version and path information
configure_file(src/compile_time.h.in "${PROJECT_BINARY_DIR}/compile_time.h")

# set KVD specific debug define
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(
    "-DKVD_DEBUG=ON"
  )
endif()

if(CMAKE_BUILD_TYPE MATCHES "(Debug|Release|RelWithDebInfo|MinSizeRel)")
  set(C_OPTS
    "-D_GLIBCXX_ASSERTIONS"
    "-D_LIBCXX_ENABLE_ASSERTIONS"
    "-fcf-protection=full"
    "-fdata-sections"
    "-ffunction-sections"
    "-fno-delete-null-pointer-checks"
    "-fno-omit-frame-pointer"
    "-fno-plt"
    "-fno-strict-aliasing"
    "-fno-strict-overflow"
    "-fstack-clash-protection"
    "-fstack-protector-strong"
    "-fstrict-flex-arrays=3"
    "-ftrivial-auto-var-init=zero"
    "-mbranch-protection=standard"
    "-mno-omit-leaf-frame-pointer"
    "-mshstk"
    "-pedantic"
    "-std=gnu17"
    "-Wall"
    "-Werror"
    "-Wextra"
    "-Wformat=2"
    "-Wformat-security"
    "-Winit-self"
    "-Wmissing-include-dirs"
    "-Wnested-externs"
    "-Wold-style-definition"
    "-Wredundant-decls"
    "-Wshadow"
    "-Wsign-compare"
    "-Wstrict-prototypes"
    "-Wundef"
    "-Wuninitialized"
    "-Wunused-parameter"
    "-Wvla"
    "-Wwrite-strings"
  )
  # check and set strict global compile flags
  foreach(FLAG IN ITEMS ${C_OPTS})
    message("Checking for compiler flag ${FLAG}")
    unset(COMPILER_SUPPORTS_FLAG CACHE)
    unset(COMPILER_SUPPORTS_FLAG)
    check_c_compiler_flag("${FLAG}" COMPILER_SUPPORTS_FLAG)
    if(COMPILER_SUPPORTS_FLAG)
      add_compile_options("${FLAG}")
    endif()
  endforeach()

  if(NOT CMAKE_C_FLAGS MATCHES "_FORTIFY_SOURCE")
    add_compile_options(
      "-D_FORTIFY_SOURCE=2"
    )
  endif()
else()
  # if CMAKE_BUILD_TYPE is neither Release nor Debug,
  # do not alter compile options
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(
    "-ggdb"
    "-Og"
  )
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
elseif(CMAKE_BUILD_TYPE MATCHES "(Release|RelWithDebInfo|MinSizeRel)")
  add_compile_options(
    "-fPIE"
    "-O2"
  )
  if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    message("Generating binary with debug symbols")
    add_compile_options("-g")
  endif()
  # IPO/LTO support
  check_ipo_supported()
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
  # if CMAKE_BUILD_TYPE is not Debug, Release, RelWithDebInfo or MinSizeRel
  # do not alter compile options
endif()

if(CMAKE_BUILD_TYPE MATCHES "(Release|RelWithDebInfo|MinSizeRel)")
  add_link_options(
    "-pie"
    "-Wl,-z,nodlopen"
    "-Wl,-z,noexecstack"
    "-Wl,-z,relro"
    "-Wl,-z,now"
    "-Wl,--as-needed"
    "-Wl,--no-copy-dt-needed-entries"
    "-Wl,--gc-sections"
  )
  if(CMAKE_BUILD_TYPE MATCHES "(Release|MinSizeRel)")
    message("Generating stripped binary")
    add_link_options("-s")
  endif()
else()
  # if CMAKE_BUILD_TYPE is not Release, RelWithDebInfo or MinSizeRel
  # do not alter link options
endif()

# distributed libraries
add_subdirectory("dist")

# the main KEBA target
add_subdirectory("src")

# link all together
target_link_libraries(kvd
  mongoose
  rax
  ${MATH_LIB}
)

# install
install(TARGETS kvd DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/cmake/Install.cmake)
