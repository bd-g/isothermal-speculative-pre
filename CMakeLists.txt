# From Instruction on "Embedding LLVM in your project" (https://llvm.org/docs/CMake.html#embedding-llvm-in-your-project)
#   and "Developing LLVM passes out of source" (https://llvm.org/docs/CMake.html#developing-llvm-passes-out-of-source)

cmake_minimum_required(VERSION 3.4.3)
project(ISPREProject)
find_package(LLVM REQUIRED CONFIG)  
               
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")        # You don't need to change ${LLVM_CMAKE_DIR} since it is already defined.
include(AddLLVM)

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

add_subdirectory(ISPRE)                                     # Add the directory which your pass lives.
