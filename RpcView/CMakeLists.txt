cmake_minimum_required (VERSION 3.0.2)

message("[RpcView]")

project(RpcView)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
# set CMAKE_PREFIX_PATH=C:\Qt\Qt5.9.1\5.9.1\msvc2015 or c:\Qt\Qt5.9.1\5.9.1\msvc2015_64
find_package(Qt5Widgets)
    
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

target_link_libraries(RpcView Qt5::Widgets $ENV{CMAKE_PREFIX_PATH}/lib/Qt5WinExtras.lib)