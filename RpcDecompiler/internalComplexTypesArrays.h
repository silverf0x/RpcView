
#ifndef _INTERNAL_COMPLEX_TYPES_ARRAYS_H_
#define _INTERNAL_COMPLEX_TYPES_ARRAYS_H_

#include <sstream>
#include <list>
#include "internalRpcDecompTypeDefs.h"



BOOL __fastcall processFixedSizeArray(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_In_	FC_TYPE fcType,
	_Inout_ ParamDesc& ParamDesc,
	_Inout_ std::list<TypeToDefine>& listAdditionalTypes,
	_Inout_ std::ostringstream& oss);

BOOL __fastcall processConformantArray(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_In_	FC_TYPE fcType,
	_Inout_ ParamDesc& ParamDesc,
	_Inout_	std::list<TypeToDefine>& listAdditionalTypes,
	_Inout_	std::ostringstream& oss);

BOOL __fastcall processConformantVaryingArray(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_In_	FC_TYPE fcType,
	_Inout_	ParamDesc& ParamDesc,
	_Inout_	std::list<TypeToDefine>& listAdditionalTypes,
	_Inout_	std::ostringstream& oss);

BOOL __fastcall processVaryingArray(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_In_	FC_TYPE fcType,
	_Inout_ ParamDesc& ParamDesc,
	_Inout_ std::list<TypeToDefine>& listAdditionalTypes,
	_Inout_ std::ostringstream& oss);

BOOL __fastcall processComplexArray(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_In_	FC_TYPE fcType,
	_Inout_ ParamDesc& ParamDesc,
	_Inout_ std::list<TypeToDefine>& listAdditionalTypes,
	_Inout_ std::ostringstream& oss);

UINT __fastcall getArrayMemorySize(
	_In_	VOID* pContext,
	_In_	RVA_T pType);




#endif//_INTERNAL_COMPLEX_TYPES_ARRAYS_H_