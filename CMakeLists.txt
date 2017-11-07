cmake_minimum_required (VERSION 3.0.2) 
 
project(RpcView)

set(RPCVIEW_VERSION_MAJOR   0)
set(RPCVIEW_VERSION_MINOR   3)
set(RPCVIEW_VERSION_RELEASE 0)

# configure a header file to pass some of the CMake settings to the source code
configure_file (
  "RpcView/RpcViewVersion.h.in"
  "${PROJECT_BINARY_DIR}/RpcViewVersion.h"
  )
  
# add the binary tree to the search path for include files
# so that we will find RpcViewVersion.h
include_directories("${PROJECT_BINARY_DIR}")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS_DEBUG "/W3 /MTd /EHsc /Zi")
set(CMAKE_C_FLAGS_DEBUG "/W4 /MTd /EHsc /Zi")

set(CMAKE_CXX_FLAGS_RELEASE "/W3 /O2 /Oi /Ot /Gy /MT /EHsc /MP")
set(CMAKE_C_FLAGS_RELEASE "/W4 /O2 /Oi /Ot /Gy /MT /EHsc /MP")

set(CMAKE_EXE_LINKER_FLAGS "/INCREMENTAL:NO /OPT:REF /OPT:ICF")

add_definitions(-D_MBCS)

add_subdirectory(RpcView)
add_subdirectory(RpcDecompiler)
add_subdirectory(RpcCore)