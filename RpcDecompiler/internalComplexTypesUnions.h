#ifndef _INTERNAL_COMPLEX_TYPES_UNIONS_H_
#define _INTERNAL_COMPLEX_TYPES_UNIONS_H_

#include <sstream>
#include "internalRpcDecompTypeDefs.h"


//--------------------------------------------------------------------------
BOOL __fastcall processNonEncapsulatedUnion(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_In_	const FC_TYPE fcType,
	_Inout_ ParamDesc& ParamDesc, 
	_Inout_ std::list<TypeToDefine>& listProcTypes,
	_Inout_ std::ostringstream& oss);


//--------------------------------------------------------------------------
BOOL __fastcall getNonEncapsulatedUnionMemorySize(
	_In_	VOID*	pContext,
	_In_	RVA_T	pType,
	_Out_	size_t* pszMemorySize,
	_In_	BOOL	bHasRangeOnConformance);


//--------------------------------------------------------------------------
BOOL __fastcall processEncapsulatedUnion(
	_In_	VOID* pContext,
	_In_	const RVA_T pType,
	_In_	FC_TYPE fcType,
	_Inout_ ParamDesc& ParamDesc,
	_Inout_	std::list<TypeToDefine>& listProcTypes,
	_Inout_	std::ostringstream& oss);


//--------------------------------------------------------------------------
BOOL __fastcall getEncapsulatedUnionMemorySize(
	_In_	VOID*	pContext,
	_In_	RVA_T	pType,
	_Out_	size_t* pszMemorySize);


//--------------------------------------------------------------------------
BOOL __fastcall defineTypeNonEncapsulatedUnion(
	_In_	VOID* pContext,
	_In_	const TypeToDefine& unionDesc,
	_Inout_ std::list<TypeToDefine>& listProcTypes,
	_Inout_ std::ostringstream& oss);


//--------------------------------------------------------------------------
BOOL __fastcall defineTypeEncapsulatedUnion(
	_In_	VOID* pContext,
	_In_	RVA_T pType, 
	_In_	const TypeToDefine& unionDesc,
	_Inout_ std::list<TypeToDefine>& listProcTypes,
	_Inout_ std::ostringstream& oss);



#endif//_INTERNAL_COMPLEX_TYPES_UNIONS_H_