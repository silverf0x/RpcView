cmake_minimum_required (VERSION 3.0.2)

message("[RpcDecompiler]")


set(RpcDecompilerFiles 
	internalComplexTypesArrays.cpp
	internalComplexTypesArrays.h
	InternalComplexTypesMisc.cpp
	InternalComplexTypesMisc.h
	internalComplexTypesPointers.cpp
	internalComplexTypesPointers.h
	internalComplexTypesStrings.cpp
	internalComplexTypesStrings.h
	internalComplexTypesStructs.cpp
	internalComplexTypesStructs.h
	internalComplexTypesUnions.cpp
	internalComplexTypesUnions.h
	internalRpcDecompiler.cpp
	internalRpcDecompiler.h
	InternalRpcDecompTypeDefs.cpp
	internalRpcDecompTypeDefs.h
	internalRpcDecompTypeDefsNew.h
	internalRpcUtils.h
	InternalsRpcUtils.cpp
	internalTypeTools.cpp
	internalTypeTools.h
	RpcDecompiler.cpp
	RpcDecompiler.h
	IdlFunction.h
	IdlFunction.cpp
	IdlType.h
	IdlType.cpp
	IdlInterface.h
	IdlInterface.cpp
    RpcDecompilerResource.rc
)

add_library(RpcDecompiler SHARED ${RpcDecompilerFiles})
add_library(RpcDecompilerStatic ${RpcDecompilerFiles})
			

