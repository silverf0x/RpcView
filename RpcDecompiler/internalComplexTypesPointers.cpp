#include "..\RpcCommon\RpcView.h"
#include "internalComplexTypesPointers.h"
#include "internalRpcDecompiler.h"
#include "internalRpcDecompTypeDefs.h"
#include "internalComplexTypesArrays.h"
#include "InternalComplexTypesMisc.h"
#include "internalRpcUtils.h"

#include <iomanip>


BOOL __fastcall processStandardPointer(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_In_	const FC_TYPE fcType,
	_Inout_ ParamDesc& ParamDesc,
	_Inout_	std::list<TypeToDefine>& listAdditionalTypes,
	_Inout_	std::ostringstream& oss)
{

	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	BYTE					bRead;
	BOOL					bResult;
	RVA_T					pComplexType = NULL;
	CommonPtrSimple_t		simplePtr;
	CommonPtrComplex_t		complexPtr;
	
	// set attributes relative to pointer
	if(ParamDesc.getuPtrLevel() == 0)  // pointer attributes seems to be applied only on first pointer level
	{
		if(fcType == FC_UP)
		{
			oss<<"[unique]";		
		}


		if(fcType == FC_RP)
		{
			oss<<"[ref]";			
		}

		if(fcType == FC_FP)
		{
			oss<<"[ptr]";
		}

		if(fcType == FC_OP)
		{
			oss<<"/*FC_OP*/";
		}
	}

	// increment pointer attribute
	ParamDesc.incPtrLevel();


	

	// read FC_POINTER_ATTRIBUTE 
	RPC_GET_PROCESS_DATA((pType+1),&bRead,sizeof(bRead));
	if(!bResult)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] processStandardPointer : unable to read process data\n");
		return FALSE;
	}

	;

	if(bRead & FC_SIMPLE_POINTER) // A pointer to a simple type or nonsized conformant string
	{
	
		// read simple ptr 
		RPC_GET_PROCESS_DATA(pType, &simplePtr, sizeof(simplePtr));
		if(!bResult)
		{
			RPC_DEBUG_FN((UCHAR*)"[ERROR] processStandardPointer : unable to read process data\n");
			return FALSE;
		}


		// special case where it's a conformant non sized string
		if((FC_TYPE)simplePtr.simple_type == FC_C_CSTRING)
		{
			oss << "[string] char";
			displayPtrLevel(ParamDesc.getuPtrLevel(), oss);
			oss<< " " << ParamDesc.getStrTypeName();
		}
		else if((FC_TYPE)simplePtr.simple_type == FC_C_WSTRING)
		{
			oss << "[string] wchar_t";
			displayPtrLevel(ParamDesc.getuPtrLevel(), oss);
			oss<< " " << ParamDesc.getStrTypeName();
		}
		else // simple type
		{
			

			bResult = processSimpleType(
				pContext, 
				(FC_TYPE)simplePtr.simple_type,
				ParamDesc,				
				oss);

			if(!bResult)
			{
				RPC_DEBUG_FN((UCHAR*)"[ERROR] processStandardPointer : unable to parsse simple type\n");
				return FALSE;
			}
		}
	}
	else // complex pointer
	{
		

		RPC_GET_PROCESS_DATA(pType, &complexPtr, sizeof(complexPtr));
		if(!bResult)
		{
			RPC_DEBUG_FN((UCHAR*)"[ERROR] processStandardPointer : unable to read process data\n");
			return FALSE;
		}

		// compute type Offset :
		pComplexType = pType + sizeof(complexPtr.pointerType) + sizeof(complexPtr.pointer_attributes) + complexPtr.offset_to_complex_description;
		//ParamDesc.setuOffset((UINT)(pComplexType - pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString)); //TODO : ....
		ParamDesc.setRva(pComplexType);

		// DEBUG
		ParamDesc.setArrayIsAttributedPointer();

		// END DEBUG

		bResult = rpcDumpType(
			pContext,
			ParamDesc,
			listAdditionalTypes,
			oss);

		if(!bResult)
		{
			RPC_DEBUG_FN((UCHAR*)"[ERROR] processStandardPointer : unable to dump complex type\n");
			return FALSE;
		}

			
	}

	// set Param size in memory
	// (set done after RpcDump type to override MemorySize of pointeed type)
	//ParamDesc.setMemorySize(POINTER_SIZE);
	
	return TRUE;
}


BOOL __fastcall processInterfacePointer(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Inout_	ParamDesc& ParamDesc,
	_Inout_	std::ostringstream& oss)
{
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	BOOL					bResult;
	BYTE					bRead;
	GUID					ifGuid;



	
	
	//read firstByte
	RPC_GET_PROCESS_DATA(++pType,&bRead,sizeof(bRead));
	if(!bResult)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] processInterfacePointer : unable to read process data\n");
		return FALSE;
	}

	

	if(bRead == FC_CONSTANT_IID)
	{
		pType++;


		// read ifGuid.Data1;
		RPC_GET_PROCESS_DATA(pType, &ifGuid, sizeof(ifGuid));

		if(!bResult)
		{
			RPC_DEBUG_FN((UCHAR*)"[ERROR] processInterfacePointer : unable to read process data\n");
			return FALSE;
		}
		pType += sizeof(ifGuid);

	

		oss<<"interface(";
		oss<<std::hex<< std::setfill('0')  << std::setw(8) <<ifGuid.Data1<<"-";
		oss<<std::hex<< std::setfill('0')  << std::setw(4) <<ifGuid.Data2<<"-";
		oss<<std::hex<< std::setfill('0')  << std::setw(4) <<ifGuid.Data3<<"-";
		for(int i=0; i<8 ; i++)
		{
			oss<<std::hex<<(int)ifGuid.Data4[i];
		}
		oss<<")* ";

	}
	else //FC_PAD-> interface pointer with idd_is()
	{
		// TODO
		oss<<"[iid_is(TODO)] FC_IP ";
	}

	oss<<ParamDesc.getStrTypeName();

	// set Param size in memory
	// (set done after RpcDump type to override MemorySize of pointeed type)
	//ParamDesc.setMemorySize(POINTER_SIZE);
	
	return TRUE;
}


BOOL __fastcall processByteCountPointer(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Inout_ ParamDesc& ParamDesc,
	_Inout_ std::list<TypeToDefine>& listAdditionalTypes,
	_Inout_	std::ostringstream& oss)
{

	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	BOOL					bResult = TRUE;
	short					shortOffset;
	ByteCountHeader_T		byteCountHeader;
	ConformanceDescr_T		confDescr;

	

	// increment pointer count
	ParamDesc.incPtrLevel();

	RPC_GET_PROCESS_DATA(pType,&byteCountHeader,sizeof(byteCountHeader));

	if(!bResult)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] processByteCountPointer : unable to read process data\n");
		return FALSE;
	}

	pType += sizeof(byteCountHeader);

	// conformance part parsing
	// byte_count is an acf attributes, so we'll comment it

	oss << "/* ACF attributes : ";
	confDescr.pType = pType;
	confDescr.confType = byte_count;

	RPC_GET_PROCESS_DATA(confDescr.pType, &confDescr.corrDesc, sizeof(confDescr.corrDesc));

	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] processByteCountPointer : unable to read process data\n");
		return FALSE;
	}

	processCorrelationDescriptor(pContext, confDescr, oss, ParamDesc);
	oss << "*/ ";

	// skip conformance part
	pType += getCorrelationDescriptorSize(robustFlagWasSet, ParamDesc.hasRangeOnConformance());

	

	// check if it's simple type or complex type
	if(byteCountHeader.simpleTypeOrPad == FC_PAD)
	{


		// COMPLEX_TYPE
		// read offset to complex type
		RPC_GET_PROCESS_DATA(pType,&shortOffset,sizeof(shortOffset));
		if(!bResult)
		{
			RPC_DEBUG_FN((UCHAR*)"[ERROR] processByteCountPointer : unable to read process data\n");
			return FALSE;
		}

	
		pType += shortOffset;
		//ParamDesc.setuOffset((UINT)(pType - pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString)); 
		ParamDesc.setRva(pType);

		//dump COMPLEX TYPE
		bResult = rpcDumpType(
			pContext,
			ParamDesc, 
			listAdditionalTypes,
			oss);

		if(!bResult)
		{
			RPC_DEBUG_FN((UCHAR*)"[ERROR] processByteCountPointer : unable to dump embeded type");
			return FALSE;
		}
		
	}
	else
	{
		// SIMPLE_TYPE
		if(!processSimpleType(pContext, (FC_TYPE) byteCountHeader.simpleTypeOrPad, ParamDesc, oss))
		{
			RPC_DEBUG_FN((UCHAR*)"[ERROR] processByteCountPointer : unable to dump simple type");
			return FALSE;
		}
	}


	// set Param size in memory
	// (set done after RpcDump type to override MemorySize of pointeed type)
	//ParamDesc.setMemorySize(POINTER_SIZE);
	
	return TRUE;
}

