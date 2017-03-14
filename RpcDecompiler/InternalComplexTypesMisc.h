#ifndef _INTERNAL_COMPLEX_TYPES_MISC_H_
#define _INTERNAL_COMPLEX_TYPES_MISC_H_

#include <sstream>
#include <list>
#include "internalRpcDecompTypeDefs.h"


//---------------------------------------------------------------------------
BOOL __fastcall processBindContext(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Inout_	ParamDesc& ParamDesc,
	_Inout_	std::list<TypeToDefine>& listProcTypes,
	_Inout_	std::ostringstream& oss);


//---------------------------------------------------------------------------
BOOL __fastcall process_FC_BLKHOLE(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Inout_	ParamDesc& ParamDesc,
	_Inout_	std::list<TypeToDefine>& listProcTypes,
	_Inout_	std::ostringstream& oss);


//---------------------------------------------------------------------------
BOOL __fastcall getFC_BLKHOLEMemorySize(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Out_	size_t* pszMemorySize,
	_In_	BOOL	bHasRangeOnConformance);


//---------------------------------------------------------------------------
BOOL __fastcall processRange(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Inout_	ParamDesc& ParamDesc,
	_Inout_	std::list<TypeToDefine>& listProcTypes,
	_Inout_	std::ostringstream& oss);


//---------------------------------------------------------------------------
BOOL __fastcall getRangeMemorySize(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Out_	size_t* pszMemorySize);

BOOL __fastcall ProcessArrayRange(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Inout_	ParamDesc& ParamDesc,
	_Inout_	std::ostringstream& temp);


//---------------------------------------------------------------------------
BOOL __fastcall processUserMarshal(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Inout_	ParamDesc& ParamDesc,
	_Inout_	std::list<TypeToDefine>& listProcTypes,
	_Inout_	std::ostringstream& oss);


//---------------------------------------------------------------------------
BOOL __fastcall getUserMarshallMemorySize(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Out_	size_t* pszMemorySize);


//---------------------------------------------------------------------------
BOOL __fastcall processPipe(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Inout_	ParamDesc& ParamDesc,
	_Inout_	std::list<TypeToDefine>& listProcTypes,
	_Inout_	std::ostringstream& oss);


//---------------------------------------------------------------------------
BOOL __fastcall getPipeMemorySize(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Out_	size_t* pszMemorySize,
	_In_	BOOL	bHasRangeOnConformance);


//---------------------------------------------------------------------------
BOOL __fastcall processCorrelationDescriptor(
	_In_	VOID* pContext,
	_In_	ConformanceDescr_T confDesc,
	_Inout_	std::ostringstream& oss,
	_Inout_	ParamDesc& paramDesc);


//---------------------------------------------------------------------------
BOOL __fastcall processCorrelationDescriptorNaked(
	_In_	VOID* pContext,
	_In_	ConformanceDescr_T confDesc,
	_Inout_	std::ostringstream& oss,
	_Inout_	ParamDesc& paramDesc);


//---------------------------------------------------------------------------
UINT __fastcall processFcExpr(
	_In_	VOID* pContext,
	_In_	RVA_T pExpr,
	_In_	ParamDesc& paramDesc,
	_In_	ConformanceDescr_T confDesc,
	_Inout_	std::ostringstream& oss);


//---------------------------------------------------------------------------
std::string __fastcall getOperatorType(
	_In_ EXPR_OPERATOR oper);


//---------------------------------------------------------------------------
BOOL __fastcall processTransmitRepresentAs(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Inout_	ParamDesc& ParamDesc,
	_Inout_	std::list<TypeToDefine>& listAdditionalTypes,
	_Inout_	std::ostringstream& oss);


//---------------------------------------------------------------------------
BOOL __fastcall getTransmitAsRepresentAsMemorySize(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Out_	size_t* pszMemorySize);


//---------------------------------------------------------------------------
UINT getCorrelationDescriptorSize(
	_In_	BOOL bRobustFlagWasSet, 
	_In_	BOOL bHasRangeOnConformance);




#endif