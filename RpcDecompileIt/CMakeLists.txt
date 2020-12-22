cmake_minimum_required (VERSION 3.0.2)

message("[RpcDecompileIt]")

add_executable(RpcDecompileIt 
    RpcDecompileIt.cpp
    DecompileItRpcStub.cpp
	RpcDecompileIt.h
)
			

target_link_libraries(RpcDecompileIt  
	RpcDecompilerStatic 
	Rpcrt4.lib 
	ntdll.lib 	
	Dbghelp.lib
)

target_include_directories(RpcDecompileIt PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/../"
)