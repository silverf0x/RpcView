#include "..\RpcCommon\RpcView.h"
#include "internalRpcDecompTypeDefs.h"
#include "internalComplexTypesArrays.h"
#include "internalTypeTools.h"
#include "internalComplexTypesPointers.h"
#include "internalRpcDecompiler.h"
#include "InternalComplexTypesMisc.h"

#include <stdio.h>


BOOL __fastcall processFixedSizeArray(
	_In_ VOID* pContext,
	_In_ RVA_T pType,
	_In_ FC_TYPE fcType,
	_Inout_ ParamDesc& ParamDesc,
	_Inout_ std::list<TypeToDefine>& listAdditionalTypes,
	_Inout_ std::ostringstream& oss)
{
	BOOL					bResult				= FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;

	ArrayDescrHeader_U		ArrayToDecode;
	UINT					ArrayMemorySize = 0;
	INT16					OffsetToEmbeddedElement = 0;

	INT						fixedArraySizes[30];
	UINT					fixedArraySizesIndex = 0;
	UINT					i = 0;

	// Variables related to embedded element (if any)
	StructMemberLayout_U	ElementMemberLayoutToDecode;
	RVA_T					pElementDescr = 0;

	SimpleStructHeader_t	embeddedStruct;

	BYTE					elementToDecode;
	SMFixedSizedArrayHeader_t SMArray;
	LGFixedSizedArrayHeader_t LGArray;

	switch(fcType)
	{
	case FC_SMFARRAY:
		RPC_GET_PROCESS_DATA(
			pType,
			&ArrayToDecode,
			sizeof(SMFixedSizedArrayHeader_t)
			);
		ArrayMemorySize = ArrayToDecode.smFixedSizedArrayHdr.totalSize;
		pType += sizeof(SMFixedSizedArrayHeader_t);
		break;
	case FC_LGFARRAY:
		RPC_GET_PROCESS_DATA(
			pType,
			&ArrayToDecode,
			sizeof(LGFixedSizedArrayHeader_t)
			);
		ArrayMemorySize = ArrayToDecode.lgFixedSizedArrayHdr.totalSize;
		pType += sizeof(LGFixedSizedArrayHeader_t); 
		break;
	default:
		bResult = FALSE;
		goto End;
	}


	//TODO Check for Pointer_layout presence
	// -->Write code here<--
	//END TODO

	RPC_GET_PROCESS_DATA(
		pType,
		&ElementMemberLayoutToDecode,
		sizeof(StructMemberLayout_U)
		);

	fixedArraySizes[fixedArraySizesIndex] = ArrayMemorySize;
	pElementDescr = pType;
	if( ElementMemberLayoutToDecode.baseTypeMemberLayout == FC_EMBEDDED_COMPLEX )
	{
		// We need the offset of the embedded structure to actually know what is inside the struct
		// Offset is positive if the struct is declared after the array we're analyzing; negative otherwise
		OffsetToEmbeddedElement = ElementMemberLayoutToDecode.nonBaseTypeMemberLayout.offsetToDescription;
		pType += sizeof(StructNonBaseTypeMemberLayout_t);
		pElementDescr = pType + OffsetToEmbeddedElement - sizeof(ElementMemberLayoutToDecode.nonBaseTypeMemberLayout.offsetToDescription);

		RPC_GET_PROCESS_DATA(
			pElementDescr,
			&elementToDecode, 
			sizeof(BYTE)
			);


		// Processing multi-dimensional array
		while((elementToDecode & 0xFF) == FC_SMFARRAY || (elementToDecode & 0xFF) == FC_LGFARRAY){
			if((elementToDecode & 0xFF) == FC_SMFARRAY){

				RPC_GET_PROCESS_DATA(
					pElementDescr,
					&SMArray, 
					sizeof(SMFixedSizedArrayHeader_t)
					);

				ArrayMemorySize = SMArray.totalSize;
				pElementDescr += sizeof(SMFixedSizedArrayHeader_t);		

			}else{ 
				if((elementToDecode & 0xFF) == FC_LGFARRAY){

					RPC_GET_PROCESS_DATA(
						pElementDescr,
						&LGArray, 
						sizeof(LGFixedSizedArrayHeader_t)
						);
					ArrayMemorySize = LGArray.totalSize;
					pElementDescr += sizeof(LGFixedSizedArrayHeader_t);
				}

			}

			fixedArraySizes[fixedArraySizesIndex] /= ArrayMemorySize;  
			fixedArraySizesIndex++;
			fixedArraySizes[fixedArraySizesIndex] = ArrayMemorySize;


			RPC_GET_PROCESS_DATA(
				pElementDescr,
				&ElementMemberLayoutToDecode,
				sizeof(StructMemberLayout_U)
				);
			if(ElementMemberLayoutToDecode.baseTypeMemberLayout == FC_EMBEDDED_COMPLEX){
				OffsetToEmbeddedElement = ElementMemberLayoutToDecode.nonBaseTypeMemberLayout.offsetToDescription;
				pElementDescr += sizeof(StructNonBaseTypeMemberLayout_t);
				pElementDescr = pElementDescr + OffsetToEmbeddedElement - sizeof(ElementMemberLayoutToDecode.nonBaseTypeMemberLayout.offsetToDescription);
				RPC_GET_PROCESS_DATA(
					pElementDescr,
					&elementToDecode, 
					sizeof(BYTE)
					);
			}else{
				elementToDecode = ElementMemberLayoutToDecode.baseTypeMemberLayout;
			}
		} // end while
	}else{ // ElementMemberLayoutToDecode.baseTypeMemberLayout == FC_EMBEDDED_COMPLEX

		RPC_GET_PROCESS_DATA(
			pType,
			&elementToDecode, 
			sizeof(BYTE)
			);
	}

	ParamDesc.setRva(pElementDescr);

	if(isSimpleType((FC_TYPE)ElementMemberLayoutToDecode.baseTypeMemberLayout))
	{
		bResult = rpcDumpType(pContext, ParamDesc, listAdditionalTypes, oss);
		if(bResult == FALSE)
		{
			RPC_DEBUG_FN((UCHAR*) "[ERROR] processFixedSizeArray : error while dumping simpleType");
			return FALSE;
		}
		fixedArraySizes[fixedArraySizesIndex] = ArrayMemorySize / getSimpleTypeMemorySize((FC_TYPE)elementToDecode);
		fixedArraySizesIndex++;
	} // isSimpleType(ElementMemberLayoutToDecode.baseTypeMemberLayout
	else{
		//UINT OffstetInTypeFormatString = 0;
		RPC_GET_PROCESS_DATA(
			pElementDescr,
			&embeddedStruct,
			sizeof(SimpleStructHeader_t)
			);
		fixedArraySizes[fixedArraySizesIndex] = ArrayMemorySize / embeddedStruct.memory_size;
		fixedArraySizesIndex++;

		bResult = rpcDumpType(pContext, ParamDesc, listAdditionalTypes, oss);
		if(bResult == FALSE)
		{
			RPC_DEBUG_FN((UCHAR*) "[ERROR] processFixedSizeArray : error while dumping non simple type");
			return FALSE;
		}

		
		// store array memory size
		//ParamDesc.setMemorySize(ArrayMemorySize);
	}

	for(i=0; i<fixedArraySizesIndex; i++)
	{
		oss<<"["<<std::dec<<fixedArraySizes[i]<<"]";
	}

	bResult = TRUE;

End:
	return(bResult);

}


BOOL __fastcall processConformantArray(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_In_	FC_TYPE fcType,
	_Inout_	ParamDesc& ParamDesc,
	_Inout_	std::list<TypeToDefine>& listAdditionalTypes,
	_Inout_	std::ostringstream& oss)
{
	BOOL					bResult = FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;

	ProcFormatStringParam_U 	paramToPrint;
	CorrelationDescriptor_U		correlationDescriptorToRead;

	ConformanceDescr_T			confDesc;

	BYTE						elementType;
	StructNonBaseTypeMemberLayout_t	elementDescription;

	BYTE						correlationType;
	BYTE						correlationOperator;
	short						correlationOffset;
	NDR_CORRELATION_FLAGS		correlationFlags = {0,0,0,0,0};

	std::ostringstream			rangeStr;

    UNREFERENCED_PARAMETER(fcType);

#ifdef DBG_DECOMP
	oss << " /* FC_CARRAY */";
#endif

	RPC_GET_PROCESS_DATA(
		pType,
		&paramToPrint, 
		sizeof(paramToPrint)
		);

	pType += sizeof(ConformantArrayHeader_t);

	// Get correlation descriptor offset in the type format string
	RPC_GET_PROCESS_DATA(
		pType,
		&correlationDescriptorToRead, 
		sizeof(correlationDescriptorToRead) // 
		);

	confDesc.pType = pType;

	if(robustFlagWasSet){
		correlationType = correlationDescriptorToRead.corrDescRobust.correlation_type;
		correlationOperator = correlationDescriptorToRead.corrDescRobust.correlation_operator;
		correlationOffset = correlationDescriptorToRead.corrDescRobust.offset;
		correlationFlags = correlationDescriptorToRead.corrDescRobust.robust_flags;

	}else{
		correlationType = correlationDescriptorToRead.corrDescNonRobust.correlation_type;
		correlationOperator = correlationDescriptorToRead.corrDescNonRobust.correlation_operator;
		correlationOffset = correlationDescriptorToRead.corrDescNonRobust.offset;
	}

	if(robustFlagWasSet)
	{
		pType += sizeof(CorrelationDescriptorRobust_t);
	}
	else
	{
		pType += sizeof(CorrelationDescriptorNonRobust_t);
	}


	confDesc.confType = size_is;
	confDesc.corrDesc = correlationDescriptorToRead.corrDescNonRobust;


	bResult = processCorrelationDescriptor(pContext, confDesc, oss,ParamDesc);
	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*) "[ERROR] processConformantArray : correlation descriptor not correctly decoded");
		return FALSE;
	}



	// Handle undocumented range attribute on conformance
	if(ParamDesc.hasRangeOnConformance() )//&& confDesc.corrDesc.correlation_type != FC_CONSTANT_CONFORMANCE && confDesc.corrDesc.correlation_type != FC_POINTER_CONFORMANCE)
	{
		bResult = ProcessArrayRange(pContext, pType, ParamDesc, rangeStr);
		if(bResult == FALSE)
		{
			RPC_DEBUG_FN((UCHAR*) "[ERROR] processConformantArray : range on conformance not correctly decoded");
			return FALSE;
		}
		oss << rangeStr.str();
		pType += sizeof(Range_t);
	}




	RPC_GET_PROCESS_DATA(
		pType,
		&elementType, 
		sizeof(BYTE));

	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] processConformantArray : unable to get process data \n");
		return FALSE;
	}

	//
	// check if a pointer layout is present
	// Warning : if a pointer layout is present it's skipped since this information seems to be redundant
	// this should be investigated
	//
	if(elementType == FC_PP)
	{
		pType += sizeof(BYTE);

		// read until FC_END is found
		while(TRUE)
		{
			RPC_GET_PROCESS_DATA(
				pType,
				&elementType, 
				sizeof(BYTE));

			if(bResult == FALSE)
			{
				RPC_DEBUG_FN((UCHAR*)"[ERROR] processConformantArray : unable to get process data \n");
				return FALSE;
			}

			pType ++ ;

			if(elementType == FC_END)
			{
				break;
			}
		}
	}


	RPC_GET_PROCESS_DATA(
		pType,
		&elementType, 
		sizeof(BYTE));

	if(elementType == FC_EMBEDDED_COMPLEX){
		RPC_GET_PROCESS_DATA(
			pType,
			&elementDescription,
			sizeof(StructNonBaseTypeMemberLayout_t)
			)
			pType += elementDescription.offsetToDescription + sizeof(elementDescription.offsetToDescription);
	}

	// ParamDesc.setuOffset((UINT)(pType - pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString));
	ParamDesc.setRva(pType);

	// Write the type of array
	if(rpcDumpType(pContext, ParamDesc, listAdditionalTypes, oss) == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] processConformantArray : unable to dump type \n");
		return FALSE;
	}



	// <bugfix_julien>
	// proposition : ne pas ajouter de [] si le tableau est préfixé par un pointeur
	if(ParamDesc.getuPtrLevel() == 0)
	{
		oss << "[]";
	}
	else
	{
#ifdef DBG_DECOMP
		oss<<"/*[] CONFORMANT_ARRAY*/";
#endif
	}

	bResult = TRUE;

	return bResult;
}


BOOL __fastcall processConformantVaryingArray(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_In_	FC_TYPE fcType,
	_Inout_	ParamDesc& ParamDesc,
	_Inout_	std::list<TypeToDefine>& listAdditionalTypes,
	_Inout_	std::ostringstream& oss)
{
	BOOL					bResult = FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;

	CorrelationDescriptor_U		correlationDescriptorToRead;

	BYTE						elementType = 0;
	StructNonBaseTypeMemberLayout_t	elementDescription;

	BYTE						correlationType;
	BYTE						correlationOperator;
	short						correlationOffset;
	NDR_CORRELATION_FLAGS		correlationFlags = {0,0,0,0,0};
	ConformanceDescr_T			confDesc;

	std::ostringstream			rangeStr;

    UNREFERENCED_PARAMETER(fcType);

	oss << " /* [DBG] FC_CVARRAY */";

	pType += sizeof(ConformantVaryingArrayHeader_t);

	// Read the first correlation descriptor
	RPC_GET_PROCESS_DATA(
		pType,
		&correlationDescriptorToRead, 
		sizeof(correlationDescriptorToRead) // 
		);

	if(robustFlagWasSet){
		correlationType = correlationDescriptorToRead.corrDescRobust.correlation_type;
		correlationOperator = correlationDescriptorToRead.corrDescRobust.correlation_operator;
		correlationOffset = correlationDescriptorToRead.corrDescRobust.offset;
		correlationFlags = correlationDescriptorToRead.corrDescRobust.robust_flags;

	}else{
		correlationType = correlationDescriptorToRead.corrDescNonRobust.correlation_type;
		correlationOperator = correlationDescriptorToRead.corrDescNonRobust.correlation_operator;
		correlationOffset = correlationDescriptorToRead.corrDescNonRobust.offset;
	}

	if(robustFlagWasSet)
	{
		pType += sizeof(CorrelationDescriptorRobust_t);
	}
	else
	{
		pType += sizeof(CorrelationDescriptorNonRobust_t);
	}

	confDesc.confType = size_is;
	confDesc.corrDesc = correlationDescriptorToRead.corrDescNonRobust;


	bResult = processCorrelationDescriptor(pContext, confDesc, oss,ParamDesc);
	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*) "[ERROR] processConformantVaryingArray : first correlation descriptor not correctly decoded");
		return FALSE;
	}

	if(ParamDesc.hasRangeOnConformance() )//&& confDesc.corrDesc.correlation_type != FC_CONSTANT_CONFORMANCE && confDesc.corrDesc.correlation_type != FC_POINTER_CONFORMANCE)
	{
		bResult = ProcessArrayRange(pContext, pType, ParamDesc, rangeStr);
		if(bResult == FALSE)
		{
			RPC_DEBUG_FN((UCHAR*) "[ERROR] processConformantArray : range on conformance not correctly decoded");
			return FALSE;
		}

		oss << rangeStr.str();
		pType += sizeof(Range_t);
	}

	// Read the second correlation descriptor
	RPC_GET_PROCESS_DATA(
		pType,
		&correlationDescriptorToRead, 
		sizeof(correlationDescriptorToRead) // 
		);

	if(robustFlagWasSet){
		correlationType = correlationDescriptorToRead.corrDescRobust.correlation_type;
		correlationOperator = correlationDescriptorToRead.corrDescRobust.correlation_operator;
		correlationOffset = correlationDescriptorToRead.corrDescRobust.offset;
		correlationFlags = correlationDescriptorToRead.corrDescRobust.robust_flags;
	}else{
		correlationType = correlationDescriptorToRead.corrDescNonRobust.correlation_type;
		correlationOperator = correlationDescriptorToRead.corrDescNonRobust.correlation_operator;
		correlationOffset = correlationDescriptorToRead.corrDescNonRobust.offset;
	}

	if(robustFlagWasSet)
	{
		pType += sizeof(CorrelationDescriptorRobust_t);
	}
	else
	{
		pType += sizeof(CorrelationDescriptorNonRobust_t);
	}

	confDesc.confType = length_is; // todo : cas du first_is
	confDesc.corrDesc = correlationDescriptorToRead.corrDescNonRobust;


	bResult = processCorrelationDescriptor(pContext, confDesc, oss,ParamDesc);
	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*) "[ERROR] processConformantVaryingArray : second correlation descriptor not correctly decoded");
		return FALSE;
	}

	if(ParamDesc.hasRangeOnConformance())
	{
		pType += sizeof(Range_t);
	}



	//
	// check if a pointer layout is present
	// Warning : if a pointer layout is present it's skipped since this information seems to be redundant
	// this should be investigated
	//

	RPC_GET_PROCESS_DATA(
		pType,
		&elementType, 
		sizeof(BYTE) // 
		);

	if(elementType == FC_PP)
	{

		pType += sizeof(BYTE);

		// read until FC_END is found
		while(TRUE)
		{
			RPC_GET_PROCESS_DATA(
				pType,
				&elementType, 
				sizeof(BYTE));

			if(bResult == FALSE)
			{
				RPC_DEBUG_FN((UCHAR*)"[ERROR] processConformantArray : unable to get process data \n");
				return FALSE;
			}

			pType ++ ;

			if(elementType == FC_END)
			{
				break;
			}


		}

	}


	RPC_GET_PROCESS_DATA(
		pType,
		&elementType, 
		sizeof(BYTE));


	if(elementType == FC_EMBEDDED_COMPLEX){
		RPC_GET_PROCESS_DATA(
			pType,
			&elementDescription,
			sizeof(StructNonBaseTypeMemberLayout_t)
			)
			pType += elementDescription.offsetToDescription + sizeof(elementDescription.offsetToDescription);
	}

	///ParamDesc.setuOffset((UINT)(pType - pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString));
	ParamDesc.setRva(pType);


	bResult = rpcDumpType(pContext, ParamDesc, listAdditionalTypes, oss); // Write the type of array
	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*) "[ERROR] processConformantVaryingArray : dump type failed");
		return FALSE;
	}

	// <bugfix_julien>
	// proposition : ne pas ajouter de [] si le tableau est préfixé par un pointeur
	if(ParamDesc.getuPtrLevel() == 0)
	{
		oss << "[]";
	}
	else
	{
		oss<<"/*[] CONFORMANT_ARRAY*/";
	}

	bResult = TRUE;

	return bResult;

}


BOOL __fastcall processVaryingArray(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_In_	FC_TYPE fcType,
	_Inout_ ParamDesc& ParamDesc,
	_Inout_ std::list<TypeToDefine>& listAdditionalTypes,
	_Inout_ std::ostringstream& oss)
{
	BOOL					bResult = FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;

	CorrelationDescriptor_U		correlationDescriptorToRead;

	UINT						argNbr = 0;
	INT							size = 0;

	ArrayDescrHeader_U			ArrayToDecode;
	UINT						ArraySize;

	BYTE						correlationType;
	BYTE						correlationOperator;
	short						correlationOffset;
	NDR_CORRELATION_FLAGS		correlationFlags = {0,0,0,0,0};

	BYTE						value;

    UNREFERENCED_PARAMETER(fcType);

	RPC_GET_PROCESS_DATA(
		pType,
		&value, 
		sizeof(BYTE) // 
		);

	switch(value){
	case FC_LGVARRAY:
		RPC_GET_PROCESS_DATA(
			pType,
			&ArrayToDecode,
			sizeof(LGVaryingArrayHeader_t)
			);
		ArraySize = ArrayToDecode.lgVaryingArrayHdr.numberOfElements;
		pType += sizeof(LGVaryingArrayHeader_t);
		break;
	case FC_SMVARRAY:
		RPC_GET_PROCESS_DATA(
			pType,
			&ArrayToDecode,
			sizeof(SMVaryingArrayHeader_t)
			);
		ArraySize = ArrayToDecode.smVaryingArrayHdr.numberOfElements;
		pType += sizeof(SMVaryingArrayHeader_t);
		break;
	default:
		RPC_DEBUG_FN((UCHAR*)"Error in PrintParamVaryingArray : bad array type");
		goto End;
	}

	// Read the variance correlation descriptor
	RPC_GET_PROCESS_DATA(
		pType,
		&correlationDescriptorToRead, 
		sizeof(correlationDescriptorToRead) // 
		);

	if(robustFlagWasSet){
		correlationType = correlationDescriptorToRead.corrDescRobust.correlation_type;
		correlationOperator = correlationDescriptorToRead.corrDescRobust.correlation_operator;
		correlationOffset = correlationDescriptorToRead.corrDescRobust.offset;
		correlationFlags = correlationDescriptorToRead.corrDescRobust.robust_flags;

	}else{
		correlationType = correlationDescriptorToRead.corrDescNonRobust.correlation_type;
		correlationOperator = correlationDescriptorToRead.corrDescNonRobust.correlation_operator;
		correlationOffset = correlationDescriptorToRead.corrDescNonRobust.offset;
	}

	pType += getCorrelationDescriptorSize(robustFlagWasSet, ParamDesc.hasRangeOnConformance());

	if(is64B)	argNbr = correlationOffset / VIRTUAL_STACK_OFFSET_GRANULARITY_64B;
	else		argNbr = correlationOffset / VIRTUAL_STACK_OFFSET_GRANULARITY_32B;

	// Only FC_CONSTANT_CONFORMANCE handled as of now.
	switch(correlationType & 0xF0){
	case FC_CONSTANT_CONFORMANCE:
		size = ((correlationOperator << 16) | correlationOffset) & 0xFFFFFF;
		break;
	default:
		break;
	}
	switch(correlationOperator){
	case FC_CALLBACK: // Callback called to evaluate expression given to last_is(). Not handled yet
		oss<<"[/*TODO first_is and/or last_is(<callback_routine_"<<std::dec<<correlationOffset<<">) */]";
		break;
	case FC_DEREFERENCE:
		oss<<"[length_is( * arg_"<<std::dec<<argNbr<<")]";
		break;
	case FC_ADD_1:
		RPC_DEBUG_FN((UCHAR*)", last_is(arg_%d)]", argNbr);
		oss<<"[last_is(arg_"<<std::dec<<argNbr<<")]";
		break;
	default:
		RPC_DEBUG_FN((UCHAR*)", length_is(%d)]", size);
		oss<<"[length_is("<<std::dec<<size<<")]";
		break;
	} // end switch(correlationType)


	//ParamDesc.setuOffset((UINT)(pType - pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString));
	ParamDesc.setRva(pType);

	bResult = rpcDumpType(pContext, ParamDesc, listAdditionalTypes, oss); // Write the type of array
	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*) "[ERROR] processVaryingArray : rpcDumpType failed");
		return FALSE;
	}

	oss<<"["<<std::dec<<ArraySize<<"]";


	// store array size in memory
	//ParamDesc.setMemorySize(ArraySize);

	bResult = TRUE;

End:
	return bResult;
}


BOOL __fastcall processComplexArray(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_In_	FC_TYPE fcType,
	_Inout_ ParamDesc& ParamDesc,
	_Inout_ std::list<TypeToDefine>& listAdditionalTypes,
	_Inout_ std::ostringstream& oss)
{
	BOOL					bResult = FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;

	CorrelationDescriptor_U		correlationDescriptorToRead;

	ComplexArrayHeader_t		complexArray;
	UINT64						arrayDescrOffset = pType - pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString;
	UINT						argNbr = 0;
	INT							size = 0;

	BYTE						correlationType;
	BYTE						correlationOperator;
	short						correlationOffset;
	NDR_CORRELATION_FLAGS		correlationFlags = {0,0,0,0,0};

	StructNonBaseTypeMemberLayout_t	elementDescription;
	UINT8						elementToDecode;

	UINT32						arraySize[30];
	UINT32						arraySizeIndex = 0;

	UINT8						varDescrHandled[15]; 
	UINT16						callbacksCalled[15];
	UINT32						i = 0;
	UINT32						j = 0;

	BOOL						isLastEmbeddedArray = FALSE;
	BOOL						lengthIs = FALSE;
	BOOL						lastIs = FALSE;
	BOOL						firstArray = TRUE;

	std::stringstream			lastIsStdString;
	lastIsStdString << "[last_is(";

	std::stringstream			lengthIsStdStr;
	lengthIsStdStr << "[length_is(";

	std::ostringstream			rangeStr; 

	ConformanceDescr_T			confDesc;

    UNREFERENCED_PARAMETER(fcType);

	memset(varDescrHandled, 0, sizeof(varDescrHandled));
	memset(callbacksCalled, -1, sizeof(callbacksCalled));



	oss << " /* [DBG] FC_BOGUS_ARRAY */ ";


	//// <DBG>

	//RPC_GET_PROCESS_DATA(
	//		pType,
	//		dbgBuf, 
	//		DBG_BUF_SIZE
	//		);

	//oss<< std::endl << "DUMP : ";
	//for(int i=0; i<DBG_BUF_SIZE; i++)
	//{

	//	oss<<" 0x"<<std::hex<<(int)dbgBuf[i];
	//	//if(i % 2 == 0 )
	//	//{
	//	//	oss<<"/* "<<i<<" */";
	//	//}

	//	//oss<<std::endl;
	//}
	//oss << std::endl;

	//// </DBG>

	oss<<"[size_is(";
	while(!isLastEmbeddedArray)																																																																				
	{
		RPC_GET_PROCESS_DATA(
			pType,
			&complexArray, 
			sizeof(ComplexArrayHeader_t) // 
			);
		pType += sizeof(ComplexArrayHeader_t);

		if(complexArray.numberOfElements == 0){
			RVA_T		pRangeDesc = pType;
			// Read the conformance correlation descriptor ( size_is, max_is ) 
			RPC_GET_PROCESS_DATA(
				pType,
				&correlationDescriptorToRead, 
				sizeof(correlationDescriptorToRead)  
				);

			if(robustFlagWasSet){
				correlationType = correlationDescriptorToRead.corrDescRobust.correlation_type;
				correlationOperator = correlationDescriptorToRead.corrDescRobust.correlation_operator;
				correlationOffset = correlationDescriptorToRead.corrDescRobust.offset;
				correlationFlags = correlationDescriptorToRead.corrDescRobust.robust_flags;
			}else{
				correlationType = correlationDescriptorToRead.corrDescNonRobust.correlation_type;
				correlationOperator = correlationDescriptorToRead.corrDescNonRobust.correlation_operator;
				correlationOffset = correlationDescriptorToRead.corrDescNonRobust.offset;
			}

			if(robustFlagWasSet)
			{
				pRangeDesc += sizeof(CorrelationDescriptorRobust_t);
			}
			else
			{
				pRangeDesc += sizeof(CorrelationDescriptorNonRobust_t);
			}

			confDesc.confType = size_is;
			confDesc.corrDesc = correlationDescriptorToRead.corrDescNonRobust;

			// decode target of ConformanceDescriptor
			bResult = processCorrelationDescriptorNaked(pContext, confDesc, oss,ParamDesc);
			if(bResult == FALSE)
			{
				RPC_ERROR_FN("processCorrelationDescriptorNaked failed\n");
				return FALSE;
			}
			if(ParamDesc.hasRangeOnConformance() && firstArray)// && confDesc.corrDesc.correlation_type != FC_CONSTANT_CONFORMANCE && confDesc.corrDesc.correlation_type != FC_POINTER_CONFORMANCE)
			{
				bResult = ProcessArrayRange(pContext, pRangeDesc, ParamDesc, rangeStr);
				if(bResult == FALSE)
				{
					RPC_ERROR_FN("ProcessArrayRange failed\n");
					return FALSE;
				}
				firstArray = FALSE;
			}

			arraySize[arraySizeIndex++] = 0;

		}else{ // complexArray.numberOfElements == 0
			arraySize[arraySizeIndex++] = complexArray.numberOfElements;	
		}

		pType += 2 * getCorrelationDescriptorSize(robustFlagWasSet, ParamDesc.hasRangeOnConformance());		// Forget about 2nd correlation descriptor, will be handled later


		RPC_GET_PROCESS_DATA(
			pType,
			&elementDescription,
			sizeof(StructNonBaseTypeMemberLayout_t)
			)
			if(elementDescription.memberType == FC_EMBEDDED_COMPLEX)
			{

				pType = pType + sizeof(StructNonBaseTypeMemberLayout_t) + elementDescription.offsetToDescription - sizeof(elementDescription.offsetToDescription);
				RPC_GET_PROCESS_DATA(
					pType,
					&elementToDecode, 
					sizeof(UINT8)
					);

				if(elementToDecode != FC_BOGUS_ARRAY) 
				{
					oss<<")]";
					isLastEmbeddedArray = TRUE;
				}else{
					oss<<",";
				}
			}else{ // elementDescription.memberType == FC_EMBEDDED_COMPLEX
				oss<<")]";
				isLastEmbeddedArray = TRUE;
			}
	} // while(!isLastEmbeddedArray)

	// Add the range if any
	oss << rangeStr.str();

	pType = (RVA_T)(arrayDescrOffset + pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString);
	isLastEmbeddedArray = FALSE;

	/****************************************************
	*
	*	
	*	Variance descriptor
	*
	*
	****************************************************/
	while(!isLastEmbeddedArray){
		RPC_GET_PROCESS_DATA(
			pType,
			&complexArray, 
			sizeof(ComplexArrayHeader_t)  
			);
		pType += sizeof(ComplexArrayHeader_t);

		// Jump over conformance descriptor
		pType += getCorrelationDescriptorSize(robustFlagWasSet, ParamDesc.hasRangeOnConformance());

		// Read variance descriptor
		RPC_GET_PROCESS_DATA(
			pType,
			&correlationDescriptorToRead, 
			sizeof(correlationDescriptorToRead)  
			);

		if(robustFlagWasSet){
			correlationType = correlationDescriptorToRead.corrDescRobust.correlation_type;
			correlationOperator = correlationDescriptorToRead.corrDescRobust.correlation_operator;
			correlationOffset = correlationDescriptorToRead.corrDescRobust.offset;
			correlationFlags = correlationDescriptorToRead.corrDescRobust.robust_flags;
		}else{
			correlationType = correlationDescriptorToRead.corrDescNonRobust.correlation_type;
			correlationOperator = correlationDescriptorToRead.corrDescNonRobust.correlation_operator;
			correlationOffset = correlationDescriptorToRead.corrDescNonRobust.offset;
		}
		pType += getCorrelationDescriptorSize(robustFlagWasSet, ParamDesc.hasRangeOnConformance());


		if(correlationType != 0xFF)
		{ // If variance descriptor is not invalid

			if(is64B)	argNbr = correlationOffset / VIRTUAL_STACK_OFFSET_GRANULARITY_64B;
			else		argNbr = correlationOffset / VIRTUAL_STACK_OFFSET_GRANULARITY_32B;

			// Only FC_CONSTANT_CONFORMANCE handled yet.
			switch(correlationType & 0xF0){

			case FC_CONSTANT_CONFORMANCE:
				size = ((correlationOperator << 16) | correlationOffset) & 0xFFFFFF;
				lastIsStdString<<std::dec<<(size-1);
				lastIs = TRUE;
				break;
			default:
				switch(correlationOperator){
				case FC_CALLBACK: // Callback called to evaluate expression given to last_is(). Not handled yet
					callbacksCalled[i] = correlationOffset;
					varDescrHandled[i] = TRUE; 
					i++;
					break;
				case FC_DEREFERENCE:
					lengthIsStdStr<<" * arg_"<<std::dec<<argNbr;
					lengthIs = TRUE;
					break;
				case FC_ADD_1:
					lastIsStdString << " arg_"<<std::dec<<argNbr;
					lastIs = TRUE;
					break;
				case FC_EXPR:
					// TODO : FC_EXPR not handled. mIDA reversing needed
					RPC_ERROR_FN("FC_EXPR todo...\n");
					break;
				default:
					lengthIsStdStr << " arg_" <<std::dec<< argNbr;
					lengthIs = TRUE;
					break;
				} // end switch(correlationType)
				break;
			}

		}// correlationType != 0xFF
		RPC_GET_PROCESS_DATA(
			pType,
			&elementDescription,
			sizeof(StructNonBaseTypeMemberLayout_t)
			)
			if(elementDescription.memberType == FC_EMBEDDED_COMPLEX){

				pType = pType + sizeof(StructNonBaseTypeMemberLayout_t) + elementDescription.offsetToDescription - sizeof(elementDescription.offsetToDescription);
				RPC_GET_PROCESS_DATA(
					pType,
					&elementToDecode, 
					sizeof(UINT8)
					);

				if(elementToDecode != FC_BOGUS_ARRAY) 
				{
					lengthIsStdStr << ")]";
					lastIsStdString << ")]";
					isLastEmbeddedArray = TRUE;
				}else{
					lengthIsStdStr << ",";
					lastIsStdString << ",";
				}
			}else{ // elementDescription.memberType == FC_EMBEDDED_COMPLEX
				lengthIsStdStr << ")]";
				lastIsStdString << ")]";
				isLastEmbeddedArray = TRUE;
			}
	} // while(!isLastEmbeddedArray)

	// Only write to file if something was written into buffers
	if(lengthIs)
	{
		oss<< lengthIsStdStr.str();//std::string(lengthIsStr);
	}

	if(lastIs)
	{	
		oss<< lastIsStdString.str();//std::string(lastIsStr);
	}
	// Write any variance callback
	oss<<" /* ";

	for(j; j<i; j++){
		if(callbacksCalled[j] != (UINT16)-1)
		{
			oss<<" callback_"<<std::dec<<callbacksCalled[j]<<" used, ";
		}
	}
	oss<<" */ ";

	ParamDesc.setRva(pType);

	bResult = rpcDumpType(pContext, ParamDesc, listAdditionalTypes, oss); // Write the type of array
	if(bResult == FALSE)
	{
		RPC_ERROR_FN("rpcDumpType failed\n");
		return FALSE;
	}
	// <bugfix_julien>
	// proposition : ne pas ajouter de [] si le tableau est préfixé par un pointeur
	if(ParamDesc.getuPtrLevel() == 0)
	{
		for(i=0; i<arraySizeIndex; i++)
		{
			if(arraySize[i] != 0)
			{
				oss<<"["<<std::dec<<arraySize[i]<<"]";
			}else{
				oss<<"[]";
			}
		}
	}

	bResult = TRUE;

	return bResult;
}


UINT __fastcall getArrayMemorySize(
	_In_	VOID* pContext,
	_In_	RVA_T pType)
{
	BOOL					bResult = FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;

	// We only need size information, which is at the same offset for both variant and fixed size array.
	SMFixedSizedArrayHeader_t	smallArray;
	LGFixedSizedArrayHeader_t	longArray;
	UINT8						arrayType;
	UINT						arraySize = 0;

	RPC_GET_PROCESS_DATA(pType, &arrayType, sizeof(UINT8));
	switch(arrayType)
	{
	case FC_SMFARRAY:
	case FC_SMVARRAY:
		RPC_GET_PROCESS_DATA(pType, &smallArray, sizeof(SMFixedSizedArrayHeader_t));
		arraySize = smallArray.totalSize;
		break;
	case FC_LGFARRAY:
	case FC_LGVARRAY:
		RPC_GET_PROCESS_DATA(pType, &longArray, sizeof(LGFixedSizedArrayHeader_t));
		arraySize = longArray.totalSize;
        break;
	default:
		arraySize = POINTER_SIZE;
		break;
	}

	return arraySize;
}
