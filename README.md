# RpcView

RpcView is a free tool to explore and decompile all RPC functionalities present on a Microsoft system.

## Compilation

Required elements to compiled the project:

* Visual Studio (currently Visual Studio 2015 community)
* CMake (at least 3.0.2)
* Qt4 (currently 4.8.6)

Before running CMake you have to set the CMAKE_PREFIX_PATH environment variable with the current Qt path, for instance:
```
set CMAKE_PREFIX_PATH=C:\Qt\4.8.6
```
Then you can run CMake to produce the project solution.
Here is an example to generate the x64 solution with Visual Studio 2015 from the ```RpcView/Build/x64``` directory:

```cmake
cmake -G"Visual Studio 14 2015 Win64" ../../
-- The C compiler identification is MSVC 19.0.24215.1
-- The CXX compiler identification is MSVC 19.0.24215.1
-- Check for working C compiler: C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/bin/x86_amd64/cl.exe
-- Check for working C compiler: C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/bin/x86_amd64/cl.exe -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working CXX compiler: C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/bin/x86_amd64/cl.exe
-- Check for working CXX compiler: C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/bin/x86_amd64/cl.exe -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Detecting CXX compile features
-- Detecting CXX compile features - done
[RpcView]
-- Looking for Q_WS_X11
-- Looking for Q_WS_X11 - not found
-- Looking for Q_WS_WIN
-- Looking for Q_WS_WIN - found
-- Looking for Q_WS_QWS
-- Looking for Q_WS_QWS - not found
-- Looking for Q_WS_MAC
-- Looking for Q_WS_MAC - not found
-- Found Qt4: C:/Qt/4.8.6/bin/qmake.exe (found version "4.8.6")
-- Target is 64 bits
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

To produce the Win32 solution with Visual Studio 2015 from the ```RpcView/Build/x86``` directory:
```cmake
cmake -G"Visual Studio 14 2015" ../../
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
-- Looking for Q_WS_X11
-- Looking for Q_WS_X11 - not found
-- Looking for Q_WS_WIN
-- Looking for Q_WS_WIN - found
-- Looking for Q_WS_QWS
-- Looking for Q_WS_QWS - not found
-- Looking for Q_WS_MAC
-- Looking for Q_WS_MAC - not found
-- Found Qt4: C:/Qt/4.8.6/bin/qmake.exe (found version "4.8.6")
-- Target is 32 bits
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