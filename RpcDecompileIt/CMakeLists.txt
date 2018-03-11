cmake_minimum_required (VERSION 3.0.2)

message("[RpcDecompileIt]")

add_executable(RpcDecompileIt 
    RpcDecompileIt.cpp
    DecompileItRpcStub.cpp
    ../RpcCommon/Misc.c
    ../RpcView/Pdb.c
)
			

target_link_libraries(RpcDecompileIt  
	RpcDecompilerStatic 
	Rpcrt4.lib 
	ntdll.lib 	
	Dbghelp.lib
)

