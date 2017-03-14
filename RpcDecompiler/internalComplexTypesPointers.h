#ifndef _INTERNAL_COMPLEX_TYPES_POINTERS_H_
#define _INTERNAL_COMPLEX_TYPES_POINTERS_H_

#include <sstream>
#include <list>
#include "internalRpcDecompTypeDefs.h"




#define POINTER_SIZE_32_BITS 4
#define POINTER_SIZE_64_BITS 8
#define POINTER_SIZE  (is64B?POINTER_SIZE_64_BITS:POINTER_SIZE_32_BITS)


//---------------------------------------------------------------------------
BOOL __fastcall processStandardPointer(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_In_	const FC_TYPE fcType,
	_Inout_ ParamDesc& ParamDesc,
	_Inout_	std::list<TypeToDefine>& listProcTypes,
	_Inout_	std::ostringstream& oss);


//---------------------------------------------------------------------------
BOOL __fastcall processInterfacePointer(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Inout_	ParamDesc& ParamDesc,
	_Inout_	std::ostringstream& oss);


//---------------------------------------------------------------------------
BOOL __fastcall processByteCountPointer(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Inout_ ParamDesc& ParamDesc,
	_Inout_ std::list<TypeToDefine>& listProcTypes,
	_Inout_	std::ostringstream& oss);




#endif//_INTERNAL_COMPLEX_TYPES_POINTERS_H_