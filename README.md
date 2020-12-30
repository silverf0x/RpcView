# RpcView

RpcView is an open-source tool to explore and decompile all RPC functionalities present on a Microsoft system.

You can download the last [automatically built release](https://ci.appveyor.com/project/silverf0x/rpcview/build/artifacts)

[![Build status](https://ci.appveyor.com/api/projects/status/o5wy6mdk16tuht70?svg=true)](https://ci.appveyor.com/project/silverf0x/rpcview)

> **Warning**: you have to install "Microsoft Visual C++ 2015 Redistributable" to use RpcView.

## How to add a new RPC runtime

Basically you have two possibilities to support a new RPC runtime (rpcrt4.dll) version:

- The easy way: just edit the RpcInternals.h file in the corresponding RpcCore directories (32 and 64-bit versions) to add your runtime version in the RPC_CORE_RUNTIME_VERSION table.
- The best way: reverse the rpcrt4.dll to define the required structures used by RpcView, e.g. RPC_SERVER, RPC_INTERFACE and RPC_ADDRESS.

Currently, the supported versions are organized as follows:

- RpcCore1 for Windows XP
- RpcCore2 for Windows 7
- RpcCore3 for Windows 8
- RpcCore4 for Windows 8.1 and 10

## Compilation

Required elements to compiled the project:

* Visual Studio (currently Visual Studio 2017 Community)
* CMake (currently 3.13.2)
* Qt5 (currently 5.15.2)

Before running CMake you have to set the CMAKE_PREFIX_PATH environment variable with the Qt **full path**, for instance (x64):
```
set CMAKE_PREFIX_PATH=C:\Qt\5.15.2\msvc2015_64\
```
Before running CMake to produce the project solution you have to create the build directories:
- ```RpcView/Build/x64``` for 64-bit targets
- ```RpcView/Build/x86``` for 32-bit targets.

Here is an example to generate the x64 solution with Visual Studio 2015 from the ```RpcView/Build/x64``` directory:

```cmake
cmake -G"Visual Studio 15 2017 Win64" ../../
-- Selecting Windows SDK version 10.0.17763.0 to target Windows 10.0.19042.
-- The C compiler identification is MSVC 19.16.27045.0
-- The CXX compiler identification is MSVC 19.16.27045.0
-- Check for working C compiler: C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/VC/Tools/MSVC/14.16.27023/bin/Hostx86/x64/cl.exe
-- Check for working C compiler: C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/VC/Tools/MSVC/14.16.27023/bin/Hostx86/x64/cl.exe -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Detecting C compile features
-- Detecting C compile features - done
-- Check for working CXX compiler: C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/VC/Tools/MSVC/14.16.27023/bin/Hostx86/x64/cl.exe
-- Check for working CXX compiler: C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/VC/Tools/MSVC/14.16.27023/bin/Hostx86/x64/cl.exe -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Detecting CXX compile features
-- Detecting CXX compile features - done
[RpcView]
[RpcDecompiler]
[RpcCore1_32bits]
[RpcCore2_32bits]
[RpcCore2_64bits]
[RpcCore3_32bits]
[RpcCore3_64bits]
[RpcCore4_32bits]
[RpcCore4_64bits]
-- Configuring done
-- Generating done
-- Build files have been written to: C:/Dev/RpcView/Build/x64
```

To produce the Win32 solution:
```
set CMAKE_PREFIX_PATH=C:\Qt\5.15.2\msvc2015
```
Then from the ```RpcView/Build/x86``` directory:
```cmake
cmake -G"Visual Studio 15 2017" ../../
-- The C compiler identification is MSVC 19.0.24215.1
-- The CXX compiler identification is MSVC 19.0.24215.1
-- Check for working C compiler: C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/bin/cl.exe
-- Check for working C compiler: C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/bin/cl.exe -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working CXX compiler: C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/bin/cl.exe
-- Check for working CXX compiler: C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/bin/cl.exe -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Detecting CXX compile features
-- Detecting CXX compile features - done
[RpcView]
[RpcDecompiler]
[RpcCore1_32bits]
[RpcCore2_32bits]
[RpcCore3_32bits]
[RpcCore4_32bits]
-- Configuring done
-- Generating done
-- Build files have been written to: C:/Dev/RpcView/Build/x86
```
Now you can compile the solution with Visual Studio or CMAKE:

```cmake
cmake --build . --config Release
```

RpcView32 binaries are produced in the ```RpcView/Build/bin/x86``` directory and RpcView64 ones in the ```RpcView/Build/bin/x64```

## Acknowledgements

* Jeremy
* Julien
* Yoanne
* Bruno
