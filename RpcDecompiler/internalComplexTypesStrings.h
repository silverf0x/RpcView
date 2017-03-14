#ifndef _INTERNAL_COMPLEX_TYPES_STRINGS_H_
#define _INTERNAL_COMPLEX_TYPES_STRINGS_H_

#include <sstream>
#include "internalRpcDecompTypeDefs.h"


//--------------------------------------------------------------------------
BOOL __fastcall processConformantString(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_In_	const FC_TYPE fcType,
	_Inout_ ParamDesc& ParamDesc,
	_Inout_	std::ostringstream& oss);


//--------------------------------------------------------------------------
BOOL __fastcall processNonConformantString(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_In_	const FC_TYPE fcType,
	_Inout_	ParamDesc& ParamDesc,
	_Inout_	std::ostringstream& oss);


//--------------------------------------------------------------------------
BOOL __fastcall processStructureString(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_In_	const FC_TYPE fcType,
	_Inout_	ParamDesc& ParamDesc,
	_Inout_	std::ostringstream& oss);



#endif//_INTERNAL_COMPLEX_TYPES_STRINGS_H_