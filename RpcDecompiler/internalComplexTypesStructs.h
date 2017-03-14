#ifndef _INTERNAL_COMPLEX_TYPES_STRUCTS_H_
#define _INTERNAL_COMPLEX_TYPES_STRUCTS_H_

#include <sstream>
#include <list>

#define POINTER_LAYOUT_ENTRY_SIZE 4


//---------------------------------------------------------------------------
BOOL __fastcall getStructureMemorySize(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Out_	size_t* pszStructureMemorySize);

//---------------------------------------------------------------------------
BOOL __fastcall processStructure(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_In_	const FC_TYPE  fcType,
	_Inout_ ParamDesc& ParamDesc,
	_Inout_	std::list<TypeToDefine>& listProcTypes,
	_Inout_	std::ostringstream& oss);


//---------------------------------------------------------------------------
BOOL __fastcall defineTypeSimpleStruct(
	_In_	 VOID* pContext,
	_In_	RVA_T pType, 
	_In_	const TypeToDefine& structureDesc,
	_Inout_ std::list<TypeToDefine>& listAdditionalType,
	_Inout_ std::ostringstream& oss);


//---------------------------------------------------------------------------
BOOL __fastcall defineTypeConformantStructure(
	_In_	VOID* pContext,
	_In_	RVA_T pType, 
	_In_	const TypeToDefine& structureDesc,
	_Inout_ std::list<TypeToDefine>& listProcTypes,
	_Inout_ std::ostringstream& oss);


//---------------------------------------------------------------------------
BOOL __fastcall defineTypeComplexStruct(
	_In_	VOID* pContext,
	_In_	RVA_T pType, 
	_In_	const TypeToDefine& structureDesc,
	_Inout_ std::list<TypeToDefine>& listProcTypes,
	_Inout_ std::ostringstream& oss);


//BOOL __fastcall defineTypeHardStruct(
//	_In_ VOID* pContext,
//	_In_ PBYTE pType, 
//	_In_ const TypeToDefine& structureDesc,
//	_Out_ std::list<TypeToDefine>& listProcTypes,
//	_Out_ std::ostringstream& oss);




#endif//_INTERNAL_COMPLEX_TYPES_STRUCTS_H_