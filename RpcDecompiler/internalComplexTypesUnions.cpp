#include <stdio.h>
#include "..\RpcCommon\RpcView.h"
#include "internalRpcDecompTypeDefs.h"
#include "internalComplexTypesUnions.h"
#include "InternalComplexTypesMisc.h"
#include "internalRpcDecompiler.h"
#include "internalTypeTools.h"
#include "internalRpcUtils.h"


BOOL __fastcall processNonEncapsulatedUnion(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_In_	const FC_TYPE fcType,
	_Inout_ ParamDesc& ParamDesc, 
	_Inout_ std::list<TypeToDefine>& listProcTypes,
	_Inout_ std::ostringstream& oss)
{
	BOOL					bResult				= FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;

    UNREFERENCED_PARAMETER(fcType);
	//NonEncapUnionHeader_t				nonEncapUnionHeaderToPrint;
	nonEncapUnion_offsetToSizeAndArmDescription_t	offsetToSizeAndArmDescription;
	UINT16											uMemorySize;

	ConformanceDescr_T		confDesc;
	UINT32					unionOffsetValueToContructTypeName = 0;
	
	RVA_T					pTypeStart;

	

	// save pUnionheader to parse it later
	pTypeStart = pType;
	

	// let's skip union header

	pType += sizeof(NonEncapUnionHeader_t);

	// we'll have to read a switch_is correlation descr
	confDesc.pType = pType;
	confDesc.confType = switch_is;

	RPC_GET_PROCESS_DATA(
		confDesc.pType,
		&(confDesc.corrDesc),
		sizeof(confDesc.corrDesc));

	pType += getCorrelationDescriptorSize(robustFlagWasSet, ParamDesc.hasRangeOnConformance());


	RPC_GET_PROCESS_DATA(
		pType,
		&offsetToSizeAndArmDescription, 
		sizeof(nonEncapUnion_offsetToSizeAndArmDescription_t)
		);	

	
	
	ParamDesc.addConformanceDescr(confDesc);
	processCorrelationDescriptor(pContext, confDesc, oss, ParamDesc);
	
	//Calculate the absolute offset where offsetToSizeAndArmDescription points
	unionOffsetValueToContructTypeName = (UINT32)((pType + offsetToSizeAndArmDescription) - pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString);




	TypeToDefine unionDesc((pType + offsetToSizeAndArmDescription) , ParamDesc);
	//unionDesc.setuOffset(unionOffsetValueToContructTypeName);
	unionDesc.setpNonEncapsulatedUnionHeader(pTypeStart);

	listProcTypes.push_back(unionDesc);

	// read union memory size
	RPC_GET_PROCESS_DATA(
		pType + offsetToSizeAndArmDescription,
		&uMemorySize, 
		sizeof(uMemorySize)
	);

	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] processNonEncapsulatedUnion : unable to dump complex type");
		return FALSE;
	}
	
	//ParamDesc.setMemorySize(uMemorySize); 

	oss<<"union union_"<< std::dec <<unionOffsetValueToContructTypeName;
	displayPtrLevel(ParamDesc.getuPtrLevel(), oss);
	oss<<" "<<ParamDesc.getStrTypeName();
	
	bResult = TRUE;		


	return bResult;
}


BOOL __fastcall getNonEncapsulatedUnionMemorySize(
	_In_	VOID*	pContext,
	_In_	RVA_T	pType,
	_Out_	size_t* pszMemorySize,
	_In_	BOOL	bHasRangeOnConformance)
{
	BOOL					bResult				= FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	nonEncapUnion_offsetToSizeAndArmDescription_t	offsetToSizeAndArmDescription;
	UINT16											uMemorySize;


	// let's skip union header and correlation descriptor
	pType += sizeof(NonEncapUnionHeader_t) + getCorrelationDescriptorSize(robustFlagWasSet, bHasRangeOnConformance);


	RPC_GET_PROCESS_DATA(
		pType,
		&offsetToSizeAndArmDescription, 
		sizeof(nonEncapUnion_offsetToSizeAndArmDescription_t)
		);	

	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] getNonEncapsulatedUnionMemorySize : unable read memory");
		return FALSE;
	}

	// read union memory size
	RPC_GET_PROCESS_DATA(
		pType + offsetToSizeAndArmDescription,
		&uMemorySize, 
		sizeof(uMemorySize)
	);

	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] getNonEncapsulatedUnionMemorySize : unable read memory");
		return FALSE;
	}
	
	*pszMemorySize = uMemorySize;



	return TRUE;
}


BOOL __fastcall processEncapsulatedUnion(
	_In_	VOID* pContext,
	_In_	const RVA_T pType,
	_In_	FC_TYPE fcType,
	_Inout_ ParamDesc& ParamDesc,
	_Inout_	std::list<TypeToDefine>& listProcTypes,
	_Inout_	std::ostringstream& oss)
{

	BOOL					bResult				= FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	//TypeToDefine unionDesc;

	EncapUnion_t			encapUnion;

    UNREFERENCED_PARAMETER(fcType);

	// get memory size
	RPC_GET_PROCESS_DATA(
		pType,
		&encapUnion,
		sizeof(encapUnion));

	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] processEncapsulatedUnion : unable to dump complex type");
		return FALSE;
	}

	// save memory size
	//ParamDesc.setMemorySize(encapUnion.sizeAndArmDescr.memorySize + ((encapUnion.switchType & ENCAP_UNION_SWITCH_MEMORY_INCREMENT_MASK) >> 0x4));
	

	// store the type in listProcTypes in order to define it later
	
	// unionDesc.m_uOffset = ParamDesc.m_uOffset;
	// unionDesc.m_fcType = fcType;
	TypeToDefine unionDesc(pType, ParamDesc);

	listProcTypes.push_back(unionDesc);
	
	oss<<"union_"<< std::dec <<ParamDesc.getRelativeOffsetFromFmtString(pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString)<<" ";
	displayPtrLevel(ParamDesc.getuPtrLevel(), oss);
	oss<<" "<<ParamDesc.getStrTypeName();
	

	return TRUE;
}


BOOL __fastcall getEncapsulatedUnionMemorySize(
	_In_	VOID*	pContext,
	_In_	RVA_T	pType,
	_Out_	size_t* pszMemorySize)
{
	BOOL					bResult				= FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	//TypeToDefine unionDesc;

	EncapUnion_t			encapUnion;

	// get memory size
	RPC_GET_PROCESS_DATA(
		pType,
		&encapUnion,
		sizeof(encapUnion));

	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] processEncapsulatedUnion : unable to dump complex type");
		return FALSE;
	}

	// save memory size
	*pszMemorySize = encapUnion.sizeAndArmDescr.memorySize + ((encapUnion.switchType & ENCAP_UNION_SWITCH_MEMORY_INCREMENT_MASK) >> 0x4);
	


	return TRUE;
}


// ------------------------------------------------------------------------------------------------
BOOL __fastcall printEachUnionMember(
	_In_	VOID* pContext,
	_In_	RVA_T pType,
	_In_	const TypeToDefine& unionDesc,
	_In_	UINT16	unionMemberCount,
	_In_	BOOL	bIsEncapsulatedUnion,
	_Inout_ std::list<TypeToDefine>& listAdditionalsType,
	_Inout_	std::ostringstream& oss)
{
	BOOL					bResult				= FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	ArmXCase_t				unionMemberToPrint;
	DefaultArmDescr_t		defaultArmToPrint = 0;
	

	//Print each member of the union + the default member
	//bResult = PrintEachUnionMember(/* in */ pRpcDecompilerCtxt, /* in */ TypeOffset, /* in */ unionEncapsulatedDescr, /* in */ unionMemberCount, _Out_ oss);
	//if(bResult == FALSE) goto End;
	for(int i = 0; i < unionMemberCount; i++)
	{
		ParamDesc		memberDesc;
		std::ostringstream ossMemberName;
	

		// each member inherits it properties from the union
		memberDesc.inheritProperties(unionDesc);


		//lire la description du membre courant de l'union
		RPC_GET_PROCESS_DATA(
			pType,
			&unionMemberToPrint, 
			sizeof(ArmXCase_t));

		// set memberDesc info
		ossMemberName << "unionMember_" << unionMemberToPrint.armCaseValue;
		memberDesc.setParamName(ossMemberName.str());

		pType += ARM_X_CASE_SIZE;
		
		if(bIsEncapsulatedUnion)
		{
			oss<<"\t\tcase ("<<unionMemberToPrint.armCaseValue<<"):\t";
		}
		else
		{
			oss<<"\t\t[case("<<unionMemberToPrint.armCaseValue<<")]\t";
		}

		if( (			unionMemberToPrint.offsetToArmDescription & FC_MAGIC_UNION_BYTE) && 
			( ((UINT16) unionMemberToPrint.offsetToArmDescription) >= MIN_UNION_SIMPLE_TYPE_ENCODE) && 
			( ((UINT16) unionMemberToPrint.offsetToArmDescription) <= MAX_UNION_SIMPLE_TYPE_ENCODE) 
			)
		{//member is a simple type
					
			bResult = processSimpleType(
				pContext, 
				(FC_TYPE) (unionMemberToPrint.offsetToArmDescription & FC_MAGIC_UNION_GET_SIMPLE_TYPE),
				memberDesc,
				oss);

			if (bResult == FALSE) goto End;	
			
			
			
		}
		else //member is a complex type
		{
			RVA_T pComplexType;
		
			//member description is localized at the relative offset unionMemberToPrint.offsetToArmDescription
			//Calculate the absolute offset where unionMemberToPrint.offsetToArmDescription points
			pComplexType = (pType											//(fin du offsetToSizeAndArmDescription courant) 
				- sizeof(unionMemberToPrint.offsetToArmDescription)	//base de l'offset à calculer 
				+ unionMemberToPrint.offsetToArmDescription);		//ajout de la valeur relative de l'offset contenue dans unionMemberToPrint.offsetToArmDescription

			
			//memberDesc.setuOffset((UINT)(pComplexType - pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString));
			memberDesc.setRva(pComplexType);

			//rpcDumpType
			bResult = rpcDumpType(
				pContext,
				memberDesc,
				listAdditionalsType,
				oss);

			

			if(bResult == FALSE)
			{
				RPC_DEBUG_FN((UCHAR*)"[printEachUnionMember] : unable to dump complex type");
				goto End;
			}
		}

		oss << ";" << std::endl;
				

	}

	// process default
	RPC_GET_PROCESS_DATA(
		pType,
		&defaultArmToPrint, 
		sizeof(DefaultArmDescr_t));	

	
	switch(defaultArmToPrint)
	{

		

	case NO_DEFAULT_ARM_DESCRIPTION:
		oss<<"\t/* no default member to print for this Union.\n\tAn exception will be raised if the switch_is value does not match any of the cases values */\n";
		RPC_DEBUG_FN((UCHAR*)"\nPrintTypeUnionNonEncapsulated : no default member to print for this Union");
		bResult = TRUE;
		break;

	case EMPTY_DEFAULT_ARM_DESCRIPTION:
		if(bIsEncapsulatedUnion)
		{
			oss<<"\t\tdefault:\n\t\t;"<<std::endl;
		}
		else
		{
			oss<<"\t\t[default]\t;"<<std::endl;
		}
		bResult = TRUE;
		break;

	default:
		if(bIsEncapsulatedUnion)
		{
			oss<<"\tdefault:"<<std::endl<<"\t\t";;
		}
		else
		{
			oss<<"\t[default]"<<std::endl<<"\t\t";;
		}
		bResult = TRUE;

		//print the default member type
		if( (			defaultArmToPrint & FC_MAGIC_UNION_BYTE )  && 
			( ((UINT16) defaultArmToPrint) >= MIN_UNION_SIMPLE_TYPE_ENCODE) && 
			( ((UINT16) defaultArmToPrint) <= MAX_UNION_SIMPLE_TYPE_ENCODE)
			)
		{
			ParamDesc defaultMemberDesc;
			defaultMemberDesc.setParamName("defaultMember");


			
			//default = simple type
			bResult = processSimpleType(
				pContext, 
				(FC_TYPE) (defaultArmToPrint & FC_MAGIC_UNION_GET_SIMPLE_TYPE),
				defaultMemberDesc,
				oss);

			if (bResult == FALSE) goto End;	
		}
		else //default = complex type
		{
			ParamDesc defaultMemberDesc;

			//member description is localized at the relative offset defaultArmToPrint
			//Calculate the absolute offset where defaultArmToPrint points
			pType = (pType								//base de l'offset à calculer 
				+ (INT16) defaultArmToPrint);			//ajout de la valeur relative de l'offset contenue dans defaultArmToPrint

			defaultMemberDesc.setParamName("defaultMember");
			//defaultMemberDesc.setuOffset((UINT)(pType - pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString));
			defaultMemberDesc.setRva(pType);
			

			//rpcDumpType
			bResult = rpcDumpType(
				pContext,
				defaultMemberDesc,
				listAdditionalsType,
				oss);

			if(bResult == FALSE)
			{
				RPC_DEBUG_FN((UCHAR*)"\n!!! source = %s ligne = %d - PrintParamOfFieldType returned bResult = %d", __FILE__, __LINE__, bResult);
				goto End;
			}
		}

		oss << ";"<<std::endl;
	}
		

	bResult = TRUE;

	End:
	return bResult;

}


BOOL __fastcall defineTypeNonEncapsulatedUnion(
	_In_	VOID* pContext,
	_In_	const TypeToDefine& unionDesc,
	_Inout_ std::list<TypeToDefine>& listAdditionalTypes,
	_Inout_ std::ostringstream& oss)
{
	BOOL					bResult				= FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	RVA_T					pType				= unionDesc.getpNonEncapuslatedUnionHeader();
	FC_TYPE					switchType;
	UnionArmSelector_t		unionArmSeltorToPrint;
	UINT16					unionMemberCount	= 0;
	NonEncapUnionHeader_t	nonEncapUnionHeader;
	//short					offsetToSizeAndArmDescr;
	
	#ifdef DBG_DECOMP

	oss << std::endl;
	oss << "\t /* FC_NON_ENCAPSULATED_UNION , offset : NO_MORE "; //<< (int)unionStrDesc.getuOffset();
	oss << ", pType : 0x" << std::hex << (int)unionDesc.getpType() << std::dec << " */";
	#endif



	// read nonEncapUnionHeader
	RPC_GET_PROCESS_DATA(
		pType,
		&nonEncapUnionHeader, 
		sizeof(nonEncapUnionHeader)
	);	

	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*)"[defineTypeNonEncapsulatedUnion] ERROR : unable to get process data");
		return FALSE;
	}

	// display switch type
	switchType = (FC_TYPE) nonEncapUnionHeader.switchType;

	
	//// skip switch_is_description (already processed by processNonEncapsulatedUnion function)
	//pType += sizeof(nonEncapUnionHeader);
	//if(robustFlagWasSet)
	//{
	//	pType += sizeof(CorrelationDescriptorRobust_t);
	//}
	//else
	//{
	//	pType += sizeof(CorrelationDescriptorRobust_t);
	//}

	//// get offset to size_and_arm description
	//RPC_GET_PROCESS_DATA(
	//	pType,
	//	&offsetToSizeAndArmDescr, 
	//	sizeof(offsetToSizeAndArmDescr)
	//);	
	//


	//pType += offsetToSizeAndArmDescr;
	

	
	//pType = pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString + unionStrDesc.getuOffset() + sizeof(INT16);
	
	pType = unionDesc.getRva();
	pType += sizeof(INT16);		// skip memory size


	//Determine the number of member of the union
	RPC_GET_PROCESS_DATA(
		pType,
		&unionArmSeltorToPrint, 
		sizeof(UnionArmSelector_t)
		);	
	pType += sizeof(UnionArmSelector_t);

	unionMemberCount = unionArmSeltorToPrint & UNION_ARM_SELTOR_NB_UNION_MEMBER_MASK;
	

	//start to print the union
	oss<<std::endl<<"\ttypedef [switch_type(";
	bResult = printSimpleType(pRpcDecompilerCtxt, switchType, oss);
	oss << ")] union union_"<<std::dec << (unionDesc.getRva() - pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString) <<std::endl;	// TODO : clean
	oss << "\t{" << std::endl;


	/// print each union member
	bResult	 = printEachUnionMember(
		pContext,
		pType,
		unionDesc,
		unionMemberCount,
		FALSE,				// type is NON encapsulated union			
		listAdditionalTypes,
		oss);

	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR]defineTypeNonEncapsulatedUnion : unable to print each union member");
	}

	oss<<"\t} union_"<< (unionDesc.getRva() - pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString) <<";"<<std::endl<<std::endl;
	


	return bResult;
}


BOOL __fastcall defineTypeEncapsulatedUnion(
	_In_	VOID* pContext,
	_In_	RVA_T pType, 
	_In_	const TypeToDefine& unionDesc,
	_Inout_ std::list<TypeToDefine>& listAdditionalTypes,
	_Inout_ std::ostringstream& oss)
{
	BOOL					bResult				= FALSE;
	RpcDecompilerCtxt_T *	pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;

	EncapUnion_t			encapUnionToPrint;
	UnionArmSelector_t		unionArmSeltorToPrint;

	UINT16					unionMemberCount = 0;
	UINT32					unionOffsetValueToContructTypeName = (UINT32)(pType - pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString);
	ParamDesc			memberDesc;
	
	RPC_GET_PROCESS_DATA(
		pType,
		&encapUnionToPrint, 
		sizeof(EncapUnion_t)
	);	

	if(encapUnionToPrint.unionType != FC_ENCAPSULATED_UNION) 
	{
		RPC_DEBUG_FN((UCHAR*)"\nPrintTypeUnionEncapsulated ERROR : unionType != FC_ENCAPSULATED_UNION"); 
		bResult = FALSE; 
		goto End;
	}
	pType += sizeof(EncapUnion_t);

	//Determine the number of member of the union
	RPC_GET_PROCESS_DATA(
		pType,
		&unionArmSeltorToPrint, 
		sizeof(UnionArmSelector_t)
		);	
	pType += sizeof(UnionArmSelector_t);

	unionMemberCount = unionArmSeltorToPrint & UNION_ARM_SELTOR_NB_UNION_MEMBER_MASK;

	//start to print the union
	oss<<"\n\ttypedef union union_"<< std::dec << unionOffsetValueToContructTypeName<<" switch (";
	
	bResult = printSimpleType(pRpcDecompilerCtxt, (FC_TYPE)(encapUnionToPrint.switchType & ENCAP_UNION_SWITCH_TYPE_TYPE_MASK) , /* in/out */ oss);
	oss<<" discrim) U"<<unionOffsetValueToContructTypeName<<"_TYPE\n\t{\n";
	
	/// print each union member
	bResult	 = printEachUnionMember(
		pContext,
		pType,
		unionDesc,
		unionMemberCount,
		TRUE,				// type is encapsulated union			
		listAdditionalTypes,
		oss);

	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*)"[defineTypeNonEncapsulatedUnion] : unable to print each union member");
	}

	oss<<"\t}union_"<<unionOffsetValueToContructTypeName<<";"<<std::endl<<std::endl;
	

End:
	return bResult;
}



