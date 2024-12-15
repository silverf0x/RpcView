#include "..\RpcCommon\RpcView.h"
#include "internalRpcDecompTypeDefs.h"
#include "internalTypeTools.h"
#include "internalComplexTypesArrays.h"
#include "internalComplexTypesPointers.h"
#include "internalComplexTypesStrings.h"
#include "internalComplexTypesStructs.h"
#include "internalComplexTypesUnions.h"
#include "internalRpcUtils.h"

#define DEFAULT_ENUM "\n\
	/* \n\
	// this is the default enum referenced by all functions and structures using an enum \n\
	typedef enum enum32_toDefine_t\n\
	{\n\
		value_0 = 0,\n\
		value_1 = 1,\n\
		//...\n\
	}enum32_toDefine_t;\n\
	*/\n"

//------------------------------------------------------------------------------
void __fastcall printDefaultEnum(
	_Inout_	std::ostringstream& oss)
{

	oss <<DEFAULT_ENUM;
}


//---------------------------------------------------------------------------
BOOL __fastcall isSimpleType(
	_In_ FC_TYPE	type)
{
	BOOL					bResult				= FALSE;

	if(	type == FC_BYTE			||
		type == FC_CHAR			||
		type == FC_SMALL		||
		type == FC_USMALL		||
		type == FC_WCHAR		||
		type == FC_SHORT		||
		type == FC_USHORT		||
		type == FC_LONG			||
		type == FC_ULONG		||
		type == FC_FLOAT		||
		type == FC_HYPER		||
		type == FC_DOUBLE		||
		type == FC_ENUM16		||
		type == FC_ENUM32		||
		type == FC_ERROR_STATUS_T	||
		type == FC_INT3264		||
		type == FC_UINT3264		||
		type == FC_IGNORE		||
		type == FC_SYSTEM_HANDLE
		)
	{
		bResult				= TRUE;
	}

	return (bResult);		
}


//-----------------------------------------------------------------------------
BOOL __fastcall printType(
	_In_	VOID* pContext, 
	_In_	TypeToDefine& typeToDefine,
	_Inout_ std::list<TypeToDefine>& listAdditionalType,
	_Inout_	std::ostringstream& oss)
{
	BOOL					bResult				= FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;

	RVA_T					pType;

	if (pRpcDecompilerCtxt == NULL) return FALSE;
	if (pRpcDecompilerCtxt->pRpcViewHelper == NULL) return FALSE;

	if(pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString == NULL) return FALSE;

	RPC_DEBUG_FN((UCHAR*)"\n\n******************** printType *********************");


	
	pType = typeToDefine.getRva();

	//RPC_GET_PROCESS_DATA(
	//	pType,
	//	&elementType,
	//	sizeof(BYTE)
	//	);

	#ifdef DBG_DECOMP
	oss << std::endl << "/*  pType :  0x"<<std::hex<<(LONGLONG)typeToDefine.getpType() ;
	if(typeToDefine.getFcType() == FC_NON_ENCAPSULATED_UNION)
	{
		oss << "  pNonEncapUnionheader : 0x"<<(LONGLONG)typeToDefine.getpNonEncapuslatedUnionHeader();
	}
	oss << std::dec<<" */"<< std::endl;
	#endif



	switch(typeToDefine.getFcType())
	{
	//
	// structures
	//
	case FC_STRUCT:
		bResult = defineTypeSimpleStruct(pContext, pType, typeToDefine, listAdditionalType, oss);
		//PrintTypeSimpleStructure(pContext, pType, typeOfContainer, listAdditionalType, oss);
		break;

	case FC_PSTRUCT:
		#ifdef DBG_DECOMP
		oss << std::endl << "\t/* process FC_PSTRUCT */" << std::endl;
		#endif
		bResult = defineTypeSimpleStruct(pContext, pType, typeToDefine, listAdditionalType, oss);
		break;


	case FC_CSTRUCT:
		bResult = defineTypeConformantStructure(pContext, pType, typeToDefine, listAdditionalType, oss);
		//PrintTypeConformantStructure(pContext, pType, typeOfContainer, listAdditionalType, oss);
		break;

	case FC_CPSTRUCT:
		oss << std::endl << "\t/* [TO_CHECK] FC_CPSTRUCT  */" << std::endl;
		bResult = defineTypeConformantStructure(pContext, pType, typeToDefine, listAdditionalType, oss);
		break;

	case FC_CVSTRUCT:
		oss << std::endl << "\t/* [ERROR] FC_CVSTRUCT are currently not implemented */" << std::endl;
		break;

	case FC_BOGUS_STRUCT:
		bResult = defineTypeComplexStruct(pContext, pType, typeToDefine, listAdditionalType, oss);
		break;
	
	case FC_HARD_STRUCT:
		oss << std::endl <<"\t /* FC_HARD_STRUCT processed as a FC_BOGUS_STRUC  */ " << std::endl;
		bResult = defineTypeComplexStruct(pContext, pType, typeToDefine, listAdditionalType, oss);
		break;
	//
	// unions
	//
	case FC_ENCAPSULATED_UNION:

		bResult = defineTypeEncapsulatedUnion(
			pContext,
			pType,
			typeToDefine,
			listAdditionalType,
			oss);
		break;
		

	case FC_NON_ENCAPSULATED_UNION:

		bResult = defineTypeNonEncapsulatedUnion(
			pContext,
			typeToDefine,
			listAdditionalType,
			oss);

		
		break;


	default:
		oss << "PrintType : bad type given. Should be Struct or Union only. given type : "<< std::hex << (int)typeToDefine.getFcType() << std::endl;
		return FALSE;
	}



	return bResult;
}


