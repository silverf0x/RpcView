#include <vector>
#include <sstream>
#include <map>

#include "..\RpcCommon\RpcView.h"
#include "internalRpcDecompTypeDefs.h"
#include "internalComplexTypesStructs.h"
#include "internalTypeTools.h"
#include "internalRpcDecompiler.h"
#include "internalRpcUtils.h"
#include "internalComplexTypesPointers.h"


BOOL __fastcall getStructureMemorySize(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_Out_	size_t* pszStructureMemorySize)
{
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	BOOL					bResult				= FALSE; 
	SimpleStructHeader_t	structHeader;	// header common to all kind of structures

	// read structure header in order to store it's memory size
	RPC_GET_PROCESS_DATA(
		pType,
		&structHeader, 
		sizeof(SimpleStructHeader_t)
		);

	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*) "[ERROR] processStructure : unable to get process data");
		return FALSE;
	}

	*pszStructureMemorySize = structHeader.memory_size;
	

	return TRUE;
}


BOOL __fastcall processStructure(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_In_	const FC_TYPE  fcType,
	_Inout_ ParamDesc& ParamDesc,
	_Inout_	std::list<TypeToDefine>& listProcTypes,
	_Inout_	std::ostringstream& oss)
{
	BOOL					bResult				= FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	
	SimpleStructHeader_t	structHeader;	// header common to all kind of structures

    UNREFERENCED_PARAMETER(fcType);

	// read structure header in order to store it's memory size
	RPC_GET_PROCESS_DATA(
		pType,
		&structHeader, 
		sizeof(SimpleStructHeader_t)
		);

	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*) "[ERROR] processStructure : unable to get process data");
		return FALSE;
	}

	//ParamDesc.setMemorySize(structHeader.memory_size);

	// declare structure param
	oss<<"struct Struct_"<< std::dec <<ParamDesc.getRelativeOffsetFromFmtString(pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString)<<"_t";
	displayPtrLevel(ParamDesc.getuPtrLevel(), oss);
	oss<<" "<<ParamDesc.getStrTypeName();

	// creation unionStructureDesc
	TypeToDefine structureDesc(pType, ParamDesc);
	
	listProcTypes.push_back(structureDesc);

	return TRUE;
}


BOOL __fastcall buildStructureOffsetMemberVector(
	_In_	VOID* pContext,
	_In_	RVA_T pMemberLayout, 
	_In_	const TypeToDefine& structureDesc,
	_Inout_	std::vector<UINT>& vectMembersOffset)
{
	BOOL					bResult				= FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	BYTE					bRead;
	UINT					uOffsetInMemberList = 0;
	
	while(1)
	{
	
		RPC_GET_PROCESS_DATA(
			pMemberLayout,
			&bRead,
			sizeof(bRead));

		if(bResult == FALSE)
		{
			RPC_DEBUG_FN((UCHAR*) "[ERROR] defineType_FC_BOGUS_STRUCT : unable to get process data");
			return FALSE;
		}

		
		if(isSimpleType((FC_TYPE)bRead)) // is this a simple type
		{
			// save member offset
			vectMembersOffset.push_back(uOffsetInMemberList);
			// increment offset counter
			uOffsetInMemberList += getSimpleTypeMemorySize((FC_TYPE)bRead);

			pMemberLayout += sizeof(bRead);
		}
		else if((FC_TYPE)bRead == FC_EMBEDDED_COMPLEX) // is this an embedded complex
		{
			StructNonBaseTypeMemberLayout_t structMemberLayout;
			//ParamDesc	paramDesc;
			RVA_T		pComplexType;
			size_t		szComplexSize = 0;
			//std::list<TypeToDefine>		listUseless;
			//std::ostringstream			ossUseless;
			
						
			RPC_GET_PROCESS_DATA(pMemberLayout, &structMemberLayout, sizeof(StructNonBaseTypeMemberLayout_t));
			if(bResult == FALSE)
			{
				RPC_DEBUG_FN((UCHAR*)"[ERROR] buildStructureOffsetMemberVector : unable to get process data");
				return FALSE;
			}

			//// dump type in order to get its size
			pComplexType = pMemberLayout + FIELD_OFFSET(StructNonBaseTypeMemberLayout_t, offsetToDescription) +	structMemberLayout.offsetToDescription;
			
			////paramDesc.setuOffset((UINT)(pComplexType - pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString));
			//paramDesc.setpType(pComplexType);
			//paramDesc.inheritProperties(structureDesc);

			//if (rpcDumpType(pContext, paramDesc, listUseless, ossUseless) == FALSE)
			//{
			//	RPC_DEBUG_FN((UCHAR*)"[ERROR] defineType_FC_BOGUS_STRUCT : unable to dump type");
			//	return FALSE;
			//}

			if(getTypeMemorySize(pContext, pComplexType, &szComplexSize, structureDesc.getHasRangeOnConformance()) == FALSE)
			{
				RPC_DEBUG_FN((UCHAR*)"[ERROR] buildStructureOffsetMemberVector : unable to get memory type size");
				return FALSE;
			}


			// save member offset
			vectMembersOffset.push_back(uOffsetInMemberList);
			// increment offset counter
			uOffsetInMemberList += (UINT)szComplexSize;

			
			pMemberLayout += sizeof(StructNonBaseTypeMemberLayout_t);
		}
		else if((FC_TYPE)bRead == FC_POINTER)	// is this a pointer
		{
			// save member offset
			vectMembersOffset.push_back(uOffsetInMemberList);
			// increment offset counter
			uOffsetInMemberList += POINTER_SIZE;
			
			pMemberLayout += sizeof(bRead);
		}
		else if((FC_TYPE)bRead >= FC_STRUCTPAD1 && (FC_TYPE)bRead <= FC_STRUCTPAD7) // is this PAD
		{
			// increment offset counter
			uOffsetInMemberList += bRead - (FC_STRUCTPAD1 - 1);

			pMemberLayout += sizeof(bRead);
		}
		else if((FC_TYPE)bRead == FC_PAD || (FC_TYPE)bRead == FC_END)	// is this the end ?
		{

			// store last offset off the structure (useful when we have a conformant array)
			vectMembersOffset.push_back(uOffsetInMemberList);
			// and exit
			break;
		}
		else
		{
			RPC_DEBUG_FN((UCHAR*)"/* [ERROR] unknow member layout : 0x%x", bRead );
			return FALSE;
		}
				
	}

	return TRUE;
}


BOOL __fastcall dumpEveryStructMember(
	_In_	VOID*							pContext,
	_In_	RVA_T							pMemberLayout,
	_In_opt_	RVA_T							pPointerLayout,
	_In_	const TypeToDefine&			structureDesc,
	_In_	const std::vector<UINT>&		vectMembersOffset,
	_Inout_ std::list<TypeToDefine>&	listAdditionalTypes,
	_Inout_ std::ostringstream&		oss,
	_In_opt_ const std::map<UINT,RVA_T>*	pMapPointerLayout = NULL)
{
	BOOL					bResult				= FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	BYTE					bRead;
	UINT					uStructMemberCounter	= 0;
	UINT					uPointerCounter			= 0;

	while(TRUE)
	{
	
		RPC_GET_PROCESS_DATA(
			pMemberLayout,
			&bRead,
			sizeof(bRead));

		if(bResult == FALSE)
		{
			RPC_DEBUG_FN((UCHAR*) "[ERROR] dumpEveryStructMember : unable to get process data");
			return FALSE;
		}




		if((FC_TYPE)bRead == FC_END)	// is this the end
		{
			break;
		}
		else if( ((FC_TYPE)bRead >= FC_STRUCTPAD1 && (FC_TYPE)bRead <= FC_STRUCTPAD7) || (FC_TYPE)bRead == FC_PAD) // is this PAD
		{
			// increment offset counter
			
			pMemberLayout += sizeof(bRead);
		}
		else if(bRead != FC_EMBEDDED_COMPLEX && 
				pMapPointerLayout != NULL &&
				pMapPointerLayout->find(vectMembersOffset[uStructMemberCounter]) != pMapPointerLayout->end()) // check if there's an entry in mapPointerLayout for current structure member
		{
			auto it = pMapPointerLayout->find(vectMembersOffset[uStructMemberCounter]);
			std::ostringstream				ossTmp;



			oss<<"\t\t";

			//set struct member name
			ossTmp << "\tStructMember"<<uStructMemberCounter;

			// create param desc object
			ParamDesc	paramDesc(ossTmp.str(), uStructMemberCounter, vectMembersOffset);
			// inherit its properties from structure
			paramDesc.inheritProperties(structureDesc);
		
			paramDesc.setRva(it->second);

			if (rpcDumpType(pContext, paramDesc, listAdditionalTypes, oss) == FALSE)
			{
				RPC_DEBUG_FN((UCHAR*)"[ERROR] dumpEveryStructMember : unable to dump mapPointerLayout associated type");
				oss << "/* [ERROR] dumpEveryStructMember : unable to dump mapPointerLayout associated type */" << std::endl;
				return FALSE;
			}

			// End of member printing
			oss<<";\n";

			uStructMemberCounter++;
			
			
			
			pMemberLayout += sizeof(bRead);	// type read should be a simple type (FC_LONG)

		}
		else if(isSimpleType((FC_TYPE)bRead)) // is this a simple type
		{
			std::stringstream ossTmp;
			ParamDesc paramDesc;

			oss<<"\t\t";

			//set struct member name
			ossTmp << "\tStructMember" << uStructMemberCounter;
			paramDesc.setParamName(ossTmp.str());
			paramDesc.setuStructMemberNum(uStructMemberCounter);
			// inherit its properties from structure
			paramDesc.inheritProperties(structureDesc);


			//simple type to print
			processSimpleType(pContext, (FC_TYPE)bRead, paramDesc, oss);

			// End of member printing
			oss<<";\n";
			uStructMemberCounter ++;
			
			pMemberLayout += sizeof(bRead);
		}
		else if((FC_TYPE)bRead == FC_EMBEDDED_COMPLEX) // is this an embedded complex
		{
			std::ostringstream				ossTmp;
			StructNonBaseTypeMemberLayout_t structMemberLayout;
			RVA_T							pComplexType;

			oss<<"\t\t";

			//set struct member name
			ossTmp << "\tStructMember"<<uStructMemberCounter;

			// create param desc object
			ParamDesc	paramDesc(ossTmp.str(), uStructMemberCounter, vectMembersOffset);
			// inherit its properties from structure
			paramDesc.inheritProperties(structureDesc);
		
						
			RPC_GET_PROCESS_DATA(pMemberLayout, &structMemberLayout, sizeof(StructNonBaseTypeMemberLayout_t));
			if(bResult == FALSE)
			{
				RPC_DEBUG_FN((UCHAR*)"[ERROR] dumpEveryStructMember : unable to get process data");
				return FALSE;
			}

			// dump type 
			pComplexType = pMemberLayout + sizeof(structMemberLayout.memberType) + sizeof(structMemberLayout.memoryPad) + structMemberLayout.offsetToDescription;
			//paramDesc.setuOffset((UINT)(pComplexType - pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString));
			paramDesc.setRva(pComplexType);

			if (rpcDumpType(pContext, paramDesc, listAdditionalTypes, oss) == FALSE)
			{
				RPC_DEBUG_FN((UCHAR*)"[ERROR] dumpEveryStructMember : unable to dump type");
				oss << "/* [ERROR] dumpEveryStructMember : unable to dump FC_EMBEDDED_COMPLEX type */" << std::endl;
				return FALSE;
			}

			// End of member printing
			oss<<";\n";

			uStructMemberCounter++;

				
			pMemberLayout += sizeof(StructNonBaseTypeMemberLayout_t);
		}
		else if((FC_TYPE)bRead == FC_POINTER)	// is this a pointer
		{
			std::ostringstream ossTmp;
			RVA_T	pPointer;

			// check if pPointerLayout has been provide
			if(pPointerLayout == NULL)
			{
				RPC_DEBUG_FN((UCHAR*)"[ERROR] dumpEveryStructMember, a pointer need to be decoded but an empty pPointerLayout has been provided\n");
				oss << "[ERROR] dumpEveryStructMember, a pointer need to be decoded but an empty pPointerLayout has been provided\n";
				return FALSE;
			}

			oss<<"\t\t";

			//set struct member name
			ossTmp << "\tStructMember"<<uStructMemberCounter;

			//create param desc object
			ParamDesc	paramDesc(ossTmp.str(), uStructMemberCounter, vectMembersOffset); 
			// inherit its properties from structure
			paramDesc.inheritProperties(structureDesc);
			
			pPointer = pPointerLayout + POINTER_LAYOUT_ENTRY_SIZE*uPointerCounter;

			//paramDesc.setuOffset((UINT)(pPointer - pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString));
			paramDesc.setRva(pPointer);

			bResult = rpcDumpType(pContext, paramDesc, listAdditionalTypes, oss);

			if(bResult == FALSE)
			{
				RPC_DEBUG_FN((UCHAR*)"[ERROR] dumpEveryStructMember : unable to dump FC_POINTER type");
				oss << "/* [ERROR] dumpEveryStructMember : unable to dump FC_POINTER type */" << std::endl;
				return FALSE;
			}
			// End of member printing
			oss<<";\n";

			// increment pointer counter
			uPointerCounter++;
			uStructMemberCounter++;
			
			
			pMemberLayout += sizeof(bRead);
		}
		else
		{
			RPC_DEBUG_FN((UCHAR*)"/* [ERROR] unknow member layout : 0x%x", bRead );
			oss << "/* [ERROR] unknow member layout : 0x" << std::hex <<(int) bRead << std::dec << "*/" << std::endl;
			return FALSE;
		}
				
	}

	return TRUE;
}



BOOL __fastcall parseNoRepeat(
	_In_	VOID*	pContext,
	_In_	RVA_T	pPointerInstanceLayout,
	_Out_	std::map<UINT,RVA_T>&	mapPointerLayout,
	_Out_	size_t*				pSzPointerInstanceLayout)
{
	BOOL					bResult				= FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;

	SingleInstancePtrToSimpleType_t		singleInstancePtr;
	RVA_T								pPtrDesc;

	RPC_GET_PROCESS_DATA(pPointerInstanceLayout, &singleInstancePtr, sizeof(singleInstancePtr));

	if(FALSE == bResult)
	{
		RPC_DEBUG_FN((UCHAR*) "[ERROR] parseNoRepeat : unable to get process data");
		return FALSE;
	}

	pPtrDesc = pPointerInstanceLayout + OFFSET_SINGLE_INSTANCE_TO_PTR_DESC;
	mapPointerLayout.insert( std::pair<UINT,RVA_T>(singleInstancePtr.pointerInstance.offsetToPtrInBuffer, pPtrDesc));
		
	// set szPointerInstanceLayout size
	*pSzPointerInstanceLayout = sizeof(singleInstancePtr);

	return TRUE;
}

BOOL __fastcall parseVariableRepeat(
	_In_	VOID*	pContext,
	_In_	RVA_T	pPointerInstanceLayout,
	_Inout_	std::map<UINT,RVA_T>&	mapPointerLayout,
	_Out_	size_t*				pSzPointerInstanceLayout)
{
	BOOL					bResult				= FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;

	VariableRepeatPtr_t		variableRepeatPtr;
	PointerInstance_t		pointerInstanceRead;
	RVA_T					pPointerInstance	= NULL;

	RPC_GET_PROCESS_DATA(pPointerInstanceLayout, &variableRepeatPtr, sizeof(variableRepeatPtr));

	if(FALSE == bResult)
	{
		RPC_DEBUG_FN((UCHAR*) "[ERROR] parseNoRepeat : unable to get process data");
		return FALSE;
	}


	pPointerInstance = pPointerInstanceLayout + sizeof(VariableRepeatPtr_t);

	for(int i=0; i<variableRepeatPtr.numberOfPointers; i++)
	{
		

		RPC_GET_PROCESS_DATA( 
				pPointerInstance, 
				&pointerInstanceRead,
				sizeof(pointerInstanceRead));

		if(FALSE == bResult)
		{
			RPC_DEBUG_FN((UCHAR*) "[ERROR] parseNoRepeat : unable to get process data");
			return FALSE;
		}

		mapPointerLayout.insert( std::pair<UINT,RVA_T>(	pointerInstanceRead.offsetToPtrInBuffer, 
														pPointerInstance + FIELD_OFFSET(PointerInstance_t, pointerDescription)));

		pPointerInstance += sizeof(pointerInstanceRead);
	}
	
		
	// set szPointerInstanceLayout size
	*pSzPointerInstanceLayout = (pPointerInstance - pPointerInstanceLayout);

	return TRUE;
}

BOOL __fastcall parseFixedRepeat()
{
	return FALSE;
}


BOOL __fastcall parsePointerLayout(
	_In_	VOID*					pContext,
	_In_	const RVA_T				pPointerLayout,
	_Inout_	std::map<UINT,RVA_T>&	mapPointerLayout,
	_Out_	size_t*					pSzPointerLayout,
	_Inout_	std::ostringstream&				oss)
{
	BOOL					bResult				= FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;

	PointerLayoutHeader_t				pointerLayoutHeader;
	RVA_T								pStream = pPointerLayout;
	BYTE								bFcType;


	*pSzPointerLayout = 0;
	//oss << std::endl << "\t/*DBG DUMP : ";
	//int i = 0;
	//while(true)
	//{
	//	BYTE bRead;

	//	RPC_GET_PROCESS_DATA((pStream+i), &bRead, sizeof(bRead));

	//	oss << std::hex << " 0x"<<(int)bRead;

	//	if(i++ == 128)
	//	{
	//		break;
	//	}
	//}
	//oss << " */";
	// read header
	RPC_GET_PROCESS_DATA(pStream, &pointerLayoutHeader, sizeof(pointerLayoutHeader));

	if(FALSE == bResult)
	{
		RPC_DEBUG_FN((UCHAR*) "[ERROR] parsePointerLayout : unable to get process data");
		return FALSE;
	}
	
	if(pointerLayoutHeader.pointerType != FC_PP)
	{
		RPC_DEBUG_FN((UCHAR*) "[ERROR] parsePointerLayout : not a pointer layout");
		oss << " // [ERROR] parsePointerLayout : not a pointer layout" << std::endl;
		return FALSE;
	}

	pStream += sizeof(pointerLayoutHeader);



	// read pointer description
	RPC_GET_PROCESS_DATA(pStream, &bFcType, sizeof(bFcType));

	if(FALSE == bResult)
	{
		RPC_DEBUG_FN((UCHAR*) "[ERROR] parsePointerLayout : unable to get process data");
		return FALSE;
	}

	//  FC_END means it's the end of pointers description
	while((FC_TYPE)bFcType != FC_END)
	{
	
		size_t	szPointerInstanceLayout = 0;

		if(bFcType == FC_NO_REPEAT)
		{
			if( parseNoRepeat(pContext, pStream, mapPointerLayout, &szPointerInstanceLayout) == FALSE)
			{
				oss << " // [ERROR] parsePointerLayout : error in parseNoRepeat ";
				return FALSE;
			}

			pStream += (RVA_T)szPointerInstanceLayout;
		}
		else if(bFcType == FC_VARIABLE_REPEAT)
		{
			if( parseVariableRepeat(pContext, pStream, mapPointerLayout, &szPointerInstanceLayout) == FALSE)
			{
				oss << " // [ERROR] parsePointerLayout : error in parseVariableRepeat ";
				return FALSE;
			}

			pStream += (RVA_T)szPointerInstanceLayout;
		}
		else if(bFcType == FC_FIXED_OFFSET)
		{
			oss << " // [ERROR] parsePointerLayout : error, parseFixedOffset not implemented";
			return FALSE;
		}
		else
		{
			oss << " // [ERROR] parsePointerLayout : unknown pointer layout type : (0x" << std::hex << bFcType << ")";
			return FALSE;
		}

		RPC_GET_PROCESS_DATA(pStream, &bFcType, sizeof(bFcType));

		if(FALSE == bResult)
		{
			RPC_DEBUG_FN((UCHAR*) "[ERROR] parsePointerLayout : unable to get process data");
			return FALSE;
		}

	}//while((FC_TYPE)bFcType != FC_END)

	// compute pointerLayout size
	*pSzPointerLayout = (size_t) ((pStream + 1) - pPointerLayout);

	return TRUE;
}


BOOL __fastcall defineTypeSimpleStruct(
	_In_	 VOID* pContext,
	_In_	RVA_T pType, 
	_In_	const TypeToDefine& structureDesc,
	_Inout_ std::list<TypeToDefine>& listAdditionalType,
	_Inout_ std::ostringstream& oss)
{
	BOOL					bResult				= FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;

	SimpleStructHeader_t	SimpleStructHeader;
	RVA_T					pMemberLayout		= NULL;
	std::map<UINT,RVA_T>	mapPointerLayout;
	UINT					uStructId			= (UINT32)(pType - pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString);



	#ifdef DBG_DECOMP
	oss << std::endl << "\t/*FC_STRUCT || FC_PSTRUCT */" ;
	#endif

	oss <<std::endl;
	oss << "\ttypedef struct Struct_" << std::dec << uStructId << "_t" << std::endl;
	oss << "\t{" << std::endl;


	RPC_GET_PROCESS_DATA(
		pType,
		&SimpleStructHeader, 
		sizeof(SimpleStructHeader)
		);

	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*) "[ERROR] defineTypeSimpleStruct : unable to get process data");
		return FALSE;
	}

	
	// check if it's a structure with pointers
	if(SimpleStructHeader.structType == FC_PSTRUCT)
	{
		RVA_T pPointerLayout = pType + sizeof(SimpleStructHeader);
		size_t	szPointerLayout = 0;

		bResult = parsePointerLayout(pContext, pPointerLayout, mapPointerLayout, &szPointerLayout,oss);

		if(bResult == FALSE)
		{
			oss << "[ERROR] an error has occured in parsePointerLayout\n";
			return FALSE;
		}


		// <DBG>
		//oss << " /* DBG : mapPointerLayout " << std::endl;

		//for(auto iter=mapPointerLayout.begin(); iter != mapPointerLayout.end(); iter++)
		//{
		//	oss << "offset : 0x" << std::hex <<(int)iter->first << ", ";
		//	oss << " pointer desc : " << std::hex << (LONGLONG) iter->second;
		//	oss << std::endl;
		//}

		//oss << " */" << std::endl;
		// </DBG>


		pMemberLayout = pPointerLayout + (RVA_T)szPointerLayout;
	}
	else // structure is a simple structure without pointer
	{
		pMemberLayout = pType + sizeof(SimpleStructHeader);
	}
	

	// building of a vector associating every member 
	// to its offset
	std::vector<UINT>		vectMembersOffset;
	if(buildStructureOffsetMemberVector(pContext, pMemberLayout, structureDesc, vectMembersOffset) == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*) "[ERROR] defineTypeSimpleStruct : unable to build structure member offset");
		return FALSE;
	}

	
	// then let's dump every struct member
	bResult = dumpEveryStructMember(
		pContext, 
		pMemberLayout, 
		NULL,		// no pointer layout
		structureDesc,
		vectMembersOffset,
		listAdditionalType,
		oss,
		&mapPointerLayout);

	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*) "[ERROR] defineTypeSimpleStruct : unable to dumpEveryStructMember");
		return FALSE;
	
	}


	//close the structure declaration
	oss<<"\t}Struct_"<<uStructId<<"_t;\n";

	return TRUE;
}


BOOL __fastcall defineTypeConformantStructure(
	_In_	VOID* pContext,
	_In_	RVA_T pType, 
	_In_	const TypeToDefine& structureDesc,
	_Inout_ std::list<TypeToDefine>& listAdditionalType,
	_Inout_ std::ostringstream& oss)
{
	BOOL					bResult				= FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;

	ConfStructHeader_t		confStructHeader;
	std::map<UINT,RVA_T>	mapPointerLayout;
	RVA_T					pMemberLayout;
	RVA_T					pArrayDescription;
	UINT					uStructId = (UINT32)(pType - pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString);
	UINT					uStructMemberCount;


	#ifdef DBG_DECOMP
	oss << std::endl << "\t/*FC_CSTRUCT*/" ;
	#endif
	oss<<"\n\ttypedef struct Struct_"<< std::dec << uStructId<<"_t\n\t{\n";
	

	RPC_GET_PROCESS_DATA(
		pType,
		&confStructHeader, 
		sizeof(confStructHeader)
		);

	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*) "[ERROR] defineType_FC_CSTRUCT : unable to get process data");
		return FALSE;
	}
	


	// compute pArrayDescription
	pArrayDescription = pType + FIELD_OFFSET(ConfStructHeader_t, offsetToArrayDescription) + confStructHeader.offsetToArrayDescription;
		//+ sizeof(confStructHeader.structType) 
		//+ sizeof(confStructHeader.alignment) 
		//+ sizeof(confStructHeader.memory_size)
		//+ confStructHeader.offsetToArrayDescription;



	// check if it's a structure with pointers
	if(confStructHeader.structType == FC_CPSTRUCT)
	{
		RVA_T pPointerLayout = pType + sizeof(confStructHeader);
		size_t	szPointerLayout = 0;

		bResult = parsePointerLayout(pContext, pPointerLayout, mapPointerLayout, &szPointerLayout,oss);

		if(bResult == FALSE)
		{
			oss << "[ERROR] an error has occured in parsePointerLayout\n";
			return FALSE;
		}

	

		pMemberLayout = pPointerLayout + (RVA_T)szPointerLayout;
	}
	else // structure is a simple structure without pointer
	{
		pMemberLayout = pType + sizeof(confStructHeader);
	}



	// first let's build a vect list associating every member 
	// to its offset
	std::vector<UINT>		vectMembersOffset;
	if(buildStructureOffsetMemberVector(pContext, pMemberLayout, structureDesc, vectMembersOffset) == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*) "[ERROR] defineType_FC_CSTRUCT : unable to build structure member offset");
		return FALSE;
	}

	
	// then let's dump every struct member
	bResult = dumpEveryStructMember(
		pContext, 
		pMemberLayout, 
		NULL,
		structureDesc,
		vectMembersOffset,
		listAdditionalType,
		oss);

	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*) "[ERROR] defineType_FC_CSTRUCT : unable to dumpEveryStructMember");
		return FALSE;
	
	}

	// dump last struct member (conformant string or array)
	{
		uStructMemberCount = (UINT) vectMembersOffset.size() - 1; // last struct member count is equal to the size of vectMemberOffset
		std::stringstream ss;
		ss << "StructMember" << uStructMemberCount ;

		ParamDesc lastStructMember(ss.str(), uStructMemberCount, vectMembersOffset); // TODO rassembler le inherit dans le constructeur
		lastStructMember.inheritProperties(structureDesc);
		lastStructMember.setRva(pArrayDescription);

		oss<<"\t";
		
		bResult = rpcDumpType(
			pContext, 
			lastStructMember, 
			listAdditionalType, 
			oss);

		oss<<";" << std::endl << "\t" ;

		if(bResult == FALSE)
		{
			RPC_DEBUG_FN((UCHAR*) "[ERROR] defineType_FC_CSTRUCT : unable to Dump last member");
			return FALSE;
		}
	}

	//close the structure declaration
	oss<<"\t}Struct_"<<uStructId<<"_t;\n";

	return TRUE;
}


BOOL __fastcall defineTypeComplexStruct(
	_In_	VOID* pContext,
	_In_	RVA_T pType, 
	_In_	const TypeToDefine& structureDesc,
	_Inout_ std::list<TypeToDefine>& listAdditionalType,
	_Inout_ std::ostringstream& oss)
{
	BOOL					bResult				= FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;

	ComplexStructHeader_t	complexStructHeader;


	RVA_T					pMemberLayout;
	RVA_T					pPointerLayout;
	RVA_T					pConformantArrayDescription = NULL;
	UINT					uStructId = (UINT32)(pType - pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString);
	UINT					uStructMemberCount;


	#ifdef DBG_DECOMP
	oss << std::endl << "\t/*FC_BOGUS_STRUCT*/" ;
	#endif
	oss<<"\n\ttypedef struct Struct_"<< std::dec << uStructId<<"_t\n\t{\n";
	

	RPC_GET_PROCESS_DATA(
		pType,
		&complexStructHeader, 
		sizeof(ComplexStructHeader_t)
		);

	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*) "[ERROR] defineType_FC_BOGUS_STRUCT : unable to get process data");
		return FALSE;
	}
	


	// compute PointerLayout
	pPointerLayout = pType 
		+ sizeof(complexStructHeader.structType) 
		+ sizeof(complexStructHeader.alignment) 
		+ sizeof(complexStructHeader.memory_size)
		+ sizeof(complexStructHeader.offsetToArrayDescription)
		+ complexStructHeader.offsetToPointerLayout;

	// compute pArrayDescription if needed
	if(complexStructHeader.offsetToArrayDescription != 0)
	{
		pConformantArrayDescription = pType 
			+ sizeof(complexStructHeader.structType) 
			+ sizeof(complexStructHeader.alignment) 
			+ sizeof(complexStructHeader.memory_size)
			+ complexStructHeader.offsetToArrayDescription;
	}


	pMemberLayout = pType + sizeof(ComplexStructHeader_t);

	// first let's build a vect list associating every member 
	// to its offset
	std::vector<UINT>		vectMembersOffset;
	if(buildStructureOffsetMemberVector(pContext, pMemberLayout, structureDesc, vectMembersOffset) == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*) "[ERROR] defineType_FC_BOGUS_STRUCT : unable to build structure member offset");
		return FALSE;
	}

	
	// then let's dump every struct member
	bResult = dumpEveryStructMember(
		pContext, 
		pMemberLayout, 
		pPointerLayout,
		structureDesc,
		vectMembersOffset,
		listAdditionalType,
		oss);

	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*) "[ERROR] defineType_FC_BOGUS_STRUCT : unable to dumpEveryStructMember");
		return FALSE;
	
	}

	// if needed let's dump conformant_array_description
	if(pConformantArrayDescription != NULL)
	{

		uStructMemberCount = (UINT) vectMembersOffset.size() - 1; // last struct member count is equal to the size of vectMemberOffset
		std::stringstream ss;
		ss << "StructMember" << uStructMemberCount ;

		ParamDesc lastStructMember(ss.str(), uStructMemberCount, vectMembersOffset);
		lastStructMember.inheritProperties(structureDesc);
		lastStructMember.setRva(pConformantArrayDescription);
		lastStructMember.setArrayIsAttributedPointer();

		oss<<"\t";
		
		bResult = rpcDumpType(
			pContext, 
			lastStructMember, 
			listAdditionalType, 
			oss);

		oss<<";" << std::endl << "\t" ;

		if(bResult == FALSE)
		{
			RPC_DEBUG_FN((UCHAR*) "[ERROR] defineType_FC_BOGUS_STRUCT : unable to Dump last member");
			return FALSE;
		}
	}

	//close the structure declaration
	oss<<"\t}Struct_"<<uStructId<<"_t;\n";

	return TRUE;
}


/*

FC_HARD_STRUCTURE layout :
	FC_HARD_STRUCTURE alignment<1> 
	memory_size<2> 
	reserved<4> 
	enum_offset<2> 
	copy_size<2> 
	mem_copy_incr<2> 
	union_description_offset<2>
	member_layout<> 
	FC_END


*/


//BOOL __fastcall defineTypeHardStruct(
//	_In_ VOID* pContext,
//	_In_ PBYTE pType, 
//	_Out_ std::list<TypeToDefine>& listProcTypes,
//	_Out_ std::ostringstream& oss)
//{
//	BOOL					bResult				= FALSE;
//	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
//
//	HardStructHeader_t		hardStructHeader;
//
//
//	PBYTE					pMemberLayout;
//	PBYTE					pEnum;
//	PBYTE					pUnion;
//
//	UINT					uStructId = (UINT32)(pType - pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString);
//
//	oss<<std::endl << "\t/* FC_HARD_STRUCT */ "<< std::endl;
//	oss<<"\ttypedef struct Struct_"<<uStructId<<"_t\n\t{\n";
//	
//
//	RPC_GET_PROCESS_DATA(
//		pType,
//		&hardStructHeader, 
//		sizeof(hardStructHeader)
//		);
//
//	if(bResult == FALSE)
//	{
//		RPC_DEBUG_FN((UCHAR*) "[ERROR] defineType_FC_HARD_STRUCT : unable to get process data");
//		return FALSE;
//	}
//	
//
//	
//	//// compute PointerLayout
//	//pPointerLayout = pType 
//	//	+ sizeof(complexStructHeader.structType) 
//	//	+ sizeof(complexStructHeader.alignment) 
//	//	+ sizeof(complexStructHeader.memory_size)
//	//	+ sizeof(complexStructHeader.offsetToArrayDescription)
//	//	+ complexStructHeader.offsetToPointerLayout;
//
//
//
//	pMemberLayout = pType + sizeof(hardStructHeader);
//
//	// first let's build a vect list associating every member 
//	// to its offset
//	std::vector<UINT>		vectMembersOffset;
//	if(buildStructureOffsetMemberVector(pContext, pMemberLayout, vectMembersOffset) == FALSE)
//	{
//		RPC_DEBUG_FN((UCHAR*) "[ERROR] defineType_FC_HARD_STRUCT  : unable to build structure member offset");
//		return FALSE;
//	}
//
//	
//	// then let's dump every struct member
//	bResult = dumpEveryStructMember(
//		pContext, 
//		pMemberLayout, 
//		NULL,
//		vectMembersOffset,
//		listProcTypes,
//		oss);
//
//	if(bResult == FALSE)
//	{
//		RPC_DEBUG_FN((UCHAR*) "[ERROR] defineType_FC_HARD_STRUCT  : unable to dumpEveryStructMember");
//		return FALSE;
//	
//	}
//
//
//	// and finaly dump last member : enum or union
//
//	//close the structure declaration
//	oss<<"\t}Struct_"<<uStructId<<"_t;\n";
//
//	return TRUE;
//}

