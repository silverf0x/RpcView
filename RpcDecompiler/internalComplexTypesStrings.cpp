#include "..\RpcCommon\RpcView.h"
#include "internalRpcDecompiler.h"
#include "internalRpcDecompTypeDefs.h"
#include "internalComplexTypesStrings.h"
#include "InternalComplexTypesMisc.h"
#include "internalComplexTypesPointers.h"
#include "internalRpcUtils.h"


BOOL __fastcall processConformantString(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_In_	const FC_TYPE fcType,
	_Inout_ ParamDesc& ParamDesc,
	_Inout_	std::ostringstream& oss)
{

	BOOL					bResult				= FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	BYTE					bFC_STRING_SIZED;
	ConformantSizedStr_t	conformantSizedStr;
	ConformanceDescr_T		confDesc;

	oss<<"[string]";


	// set Param size in memory
	//ParamDesc.setMemorySize(POINTER_SIZE);
	
	
	// check if string contains FC_STRING_SIZED (conformance descriptor)
	RPC_GET_PROCESS_DATA(pType+1,&bFC_STRING_SIZED,sizeof(bFC_STRING_SIZED));
	if(!bResult)
	{
		return FALSE;
	}

	if(bFC_STRING_SIZED == FC_STRING_SIZED)
	{
		// read conformance sized string
		RPC_GET_PROCESS_DATA(
			pType,
			&conformantSizedStr, 
			SIZE_OF_CONFORMANT_SIZED_STR);
		if(!bResult)
		{
			return FALSE;
		}

		pType += FIELD_OFFSET(ConformantSizedStr_t, conformance_description);

		// build conformance descriptor
		confDesc.confType = size_is;
		confDesc.corrDesc = conformantSizedStr.conformance_description.corrDescNonRobust;
		confDesc.pType = pType ;

		// print correlation desc
		processCorrelationDescriptor(pContext, confDesc, oss, ParamDesc);

		if(robustFlagWasSet)
		{
			pType += sizeof(CorrelationDescriptorRobust_t);
		}
		else
		{
			pType += sizeof(CorrelationDescriptorNonRobust_t);
		}

		if(ParamDesc.hasRangeOnConformance())
		{
			ProcessArrayRange(pContext, pType, ParamDesc, oss);
			pType += sizeof(Range_t);
		}
		
	}

	
	if(fcType == FC_C_CSTRING)
	{
		oss<<" char";
	}
	else  // FC_C_WSTRING
	{
		oss<<" wchar_t";
	}

	displayPtrLevel(ParamDesc.getuPtrLevel(), oss);
	oss << " ";
	oss<<ParamDesc.getStrTypeName();

	return TRUE;
}


BOOL __fastcall processNonConformantString(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_In_	const FC_TYPE fcType,
	_Inout_	ParamDesc& ParamDesc,
	_Inout_	std::ostringstream& oss)
{

	BOOL					bResult				= FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	NonConformantStr_t		nonConformantStrRead;

    UNREFERENCED_PARAMETER(fcType);

	// Read NonConformant string structure in order to know its type
	RPC_GET_PROCESS_DATA(
		pType,
		&nonConformantStrRead, 
		sizeof(nonConformantStrRead)
		);

	oss<<"[string]";

	if(nonConformantStrRead.stringType == FC_CSTRING)
	{
		oss<<"char";
	}
	else // Type is FC_WSTRING
	{
		oss<<" wchar_t";
	}

	displayPtrLevel(ParamDesc.getuPtrLevel(), oss);
	oss << " ";
	oss<<ParamDesc.getStrTypeName()<<"["<<nonConformantStrRead.string_size<<"]";

	
	bResult = TRUE;

	return(bResult);
}


//--------------------------------------------------------------------------
BOOL __fastcall processStructureString(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_In_	const FC_TYPE fcType,
	_Inout_	ParamDesc& ParamDesc,
	_Inout_	std::ostringstream& oss)
{
	//NonConfStructStr_t		nonConfStructStr;	// FC_SSTRING
	//ConfStructStr_t			confStructStr;		// FC_C_SSTRING
	//ConfSizedStructStr_t	confSizedStrucStr;	// FC_C_SSTRING sized
	
    UNREFERENCED_PARAMETER(pType);
    UNREFERENCED_PARAMETER(pContext);

	switch(fcType)
	{
	case FC_SSTRING:
		oss<<"[string] FC_SSTRING  /* TO BE COMPLETED */" <<ParamDesc.getStrTypeName();
		break;

	case FC_C_SSTRING:
		oss<<"[string] FC_C_SSTRING /* TO BE COMPLETED */ "<<ParamDesc.getStrTypeName();
		break;

	default:
		return FALSE;
	}
	
	return TRUE;
}