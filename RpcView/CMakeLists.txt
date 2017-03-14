cmake_minimum_required (VERSION 3.0.2)

message("[RpcView]")

project(RpcView)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
# set CMAKE_PREFIX_PATH=c:\Qt\4.8.6
find_package(Qt4 REQUIRED QtGui QtCore)
    
add_executable(
	RpcView
    
    "${PROJECT_BINARY_DIR}/../RpcViewVersion.h"
    DecompilationWidget.cpp
    EndpointsWidget.cpp
    FilterWidget.cpp
    IdlHighlighter.cpp
    InterfaceInfoWidget.cpp
    InterfacesWidget.cpp
    MainWindow.cpp
    ../RpcCommon/Misc.c
    ../RpcCommon/ntdll.c
    Pdb.c
    ProceduresWidget.cpp
    ProcessInfoWidget.cpp
    ProcessWidget.cpp
    
    ConfigurationVisitor.cpp
    EndpointSelectedVisitor.cpp
    InitViewsVisitor.cpp
    InterfaceSelectedVisitor.cpp
    ProcessSelectedVisitor.cpp
    RefreshVisitor.cpp

    RpcCoreManager.c
	RpcView.cpp
    RpcViewResource.rc
    )

if(${CMAKE_GENERATOR} MATCHES "Win64")
    message(STATUS "Target is 64 bits")
    target_link_libraries(RpcView ../../../Qt/lib/x64/QtGui ../../../Qt/lib/x64/QtCore)
    target_link_libraries(RpcView Qt4::QtGui Qt4::QtCore)
else(${CMAKE_GENERATOR} MATCHES "Win64")
    message(STATUS "Target is 32 bits")
    target_link_libraries(RpcView ../../../Qt/lib/x86/QtGui ../../../Qt/lib/x86/QtCore)
    target_link_libraries(RpcView Qt4::QtGui Qt4::QtCore)
endif(${CMAKE_GENERATOR} MATCHES "Win64")