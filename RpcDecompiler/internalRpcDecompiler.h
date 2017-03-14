#ifndef _INTERNAL_RPC_DECOMPILER_H_
#define _INTERNAL_RPC_DECOMPILER_H_

#include <list>

#include "internalRpcDecompTypeDefs.h"


//#define DBG_DECOMP


// ------------------------------------------------------------------------------------------------
BOOL __fastcall getAllTypesSortedInAList(
	_In_	VOID* pContext, 
	_In_	std::list<TypeToDefine> & listTypesToDefine,
	_Inout_ std::list<TypeToDefine>& listAllTypesSorted,
	_Inout_ std::ostringstream& oss);


// ---------------------------------------------------------------------------------------------
BOOL __fastcall dumpTypeList(
	_In_	VOID* pContext, 
	_In_	std::list<TypeToDefine> & listTypesToDefine,
	_Inout_ std::ostringstream& oss);


// ------------------------------------------------------------------------------------------------
BOOL __fastcall RpcDecompilerPrintAllTypesInList(
	_In_	VOID* pContext, 
	_In_	std::list<TypeToDefine> & listProcTypes, 
	_Out_	std::ostringstream& oss);


// ------------------------------------------------------------------------------------------------
BOOL	__fastcall RpcDecompilerDecodeAndPrintPrototypeReturnType(
	_In_	VOID* pContext, 
	_In_	UINT ProcIndex, 
	_Out_	UINT * paramOffset,
	_Out_	UINT * sizeOfProcDescr,
	_Inout_ IdlFunctionDesc& IdlFunctionDesc,
	_Inout_	std::list<TypeToDefine>& listProcType,
	_Inout_	std::ostringstream& oss);


//------------------------------------------------------------------------------
BOOL __fastcall RpcDecompilerPrintPrototypeName(
	_In_	VOID* pContext, 
	_In_	UINT ProcIndex,
	_Inout_	std::ostringstream& oss);


//------------------------------------------------------------------------------
BOOL __fastcall RpcDecompilerGetReturnParamInfo(
	_In_	VOID* pContext, 
	_In_	UINT paramOffset, 
	_In_	ParamID_E paramDescrFormat, 
	_Out_	BOOL * isReturnParam);


// ------------------------------------------------------------------------------------------------
DWORD __fastcall getSimpleTypeMemorySize(_In_ FC_TYPE fcType);


// ------------------------------------------------------------------------------------------------
BOOL __fastcall processSimpleType(
	_In_	VOID* pContext,
	_In_	FC_TYPE  fcType,
	_Inout_	ParamDesc& paramDesc,
	_Inout_	std::ostringstream& oss);


//-----------------------------------------------------------------------------
BOOL __fastcall printSimpleType(
	_In_	VOID* pContext, 
	_In_	FC_TYPE  fcType,
	_Inout_ std::ostringstream& oss);


//------------------------------------------------------------------------------
BOOL __fastcall rpcDumpType(
	_In_	VOID* pContext,
	_In_	ParamDesc& paramDesc,
	_Inout_	std::list<TypeToDefine>& listProcTypes,
	_Inout_	std::ostringstream& oss);

//-----------------------------------------------------------------------------
BOOL __fastcall getTypeMemorySize(
	_In_	VOID*	pContext,
	_In_	RVA_T	pType,
	_Out_	size_t*	pszMemorySize,
	_In_	BOOL	bHasRangeOnConformance);


// ----------------------------------------------------------------------------------------------
BOOL __fastcall RpcDecompilerPrintParam(
	_In_	VOID* pContext, 
	_In_	UINT paramOffset, 
	_In_	ParamID_E paramDescrFormat, 
	_Out_	UINT * paramSizeInBytes,
	_In_	const IdlFunctionDesc& IdlFunctionDesc,
	_Inout_	std::list<TypeToDefine>& listProcType,
	_Inout_	std::ostringstream& oss);

#endif//_INTERNAL_RPC_DECOMPILER_H_