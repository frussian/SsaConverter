cmake_minimum_required(VERSION 3.15)
project(lab4)

set(CMAKE_CXX_STANDARD 14)

find_package(LLVM 11 REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "CMAKE_ROOT ${CMAKE_ROOT}")
message(STATUS "CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}")
message(STATUS "LLVM_FOUND ${LLVM_FOUND}")
message(STATUS "LLVM_DIR ${LLVM_DIR}")
message(STATUS "LLVM_INCLUDE_DIRS: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM_LIBS: ${llvm_libs}")
message(STATUS "LLVM_DEFINITIONS: ${LLVM_DEFINITIONS}")

message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
if (NOT LLVM_ENABLE_RTTI)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()
# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(/usr/include/llvm-11)

add_definitions(${LLVM_DEFINITIONS})

# Now build our tools
add_executable(lab4 main.cpp Lexer.cpp Lexer.h Parser.cpp Parser.h Semantics.cpp Semantics.h Codegen.cpp Codegen.h CFG.cpp CFG.h BasicBlock.cpp BasicBlock.h)
# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(
        llvm_libs
        Analysis
        Core
        ExecutionEngine
        InstCombine
        Object
        OrcJIT
        RuntimeDyld
        ScalarOpts
        Support
        TransformUtils
        native
        irreader
)

# Link against LLVM libraries
target_link_libraries(lab4 ${llvm_libs})