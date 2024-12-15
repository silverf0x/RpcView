#include "internalRpcDecompTypeDefs.h"
#include "internalRpcDecompiler.h"
#include "internalRpcUtils.h"
#include "internalComplexTypesStrings.h"
#include "internalComplexTypesArrays.h"
#include "internalComplexTypesPointers.h"
#include "InternalComplexTypesMisc.h"
#include "internalComplexTypesUnions.h"
#include "internalComplexTypesStructs.h"
#include <stdio.h>
#include <sstream>
#include <iostream>

#include "internalTypeTools.h"




///\warning		Global variable is64B to set according to the real RPC binary information to decompile. The code design is to be adapted.
BOOL is64B;

//#ifdef _AMD64_
//boolean is64B = TRUE;
//#else
//boolean is64B = FALSE;
//#endif

///\warning		Global variable robustFlagWasSet to set according to the real RPC binary information to decompile. The code design is to be adapted.
/// robustFlagWasSet to set according to the real environment (/robust used or not, see following line)
///	robustFlagWasSet could be set to the w2kHeaderToDecode.interpreter_opt_flag2.HasNewCorrDesc value read from a procedure descriptor.
boolean robustFlagWasSet;





// ------------------------------------------------------------------------------------------------
BOOL __fastcall getAllTypesSortedInAList(
	_In_	VOID* pContext, 
	_In_	std::list<TypeToDefine> & listTypesToDefine,
	_Inout_ std::list<TypeToDefine>& listAllTypesSorted,
	_Inout_ std::ostringstream& oss)
{
	
	
	BOOL	bResult;

	listTypesToDefine.sort();
	listTypesToDefine.unique();

	while(!listTypesToDefine.empty())
	{
		// list used to store addition type that might be embeded in current type
		std::list<TypeToDefine> listAdditionalType;
		
		TypeToDefine TypeToDefineToDefine = listTypesToDefine.front();

		// remove first element from list
		listTypesToDefine.pop_front();

		// print it 
		bResult = printType(
			pContext, 
			TypeToDefineToDefine, 
			listAdditionalType, 
			oss);

		// if we're unable to Print type we abort
		if(bResult == FALSE)
		{
			return FALSE;
		}

		listAllTypesSorted.push_back(TypeToDefineToDefine);

		// merge listTypesToDefine and ListAdditionalTypes
		if(!listAdditionalType.empty())
		{
			//listTypesToDefine.merge(listAdditionalType);
			listTypesToDefine.splice(listTypesToDefine.end(), listAdditionalType);
		}

		// remove from listTypesToDefine elements that already are in 
		for(auto iter = listAllTypesSorted.begin(); iter != listAllTypesSorted.end(); iter++)
		{
			 listTypesToDefine.remove(*iter);
		}
				

		listTypesToDefine.sort();
		listTypesToDefine.unique();

	}

	listAllTypesSorted.unique();
	listAllTypesSorted.sort();


	return TRUE;
}


// ---------------------------------------------------------------------------------------------
BOOL __fastcall dumpTypeList(
	_In_	VOID* pContext, 
	_In_	std::list<TypeToDefine> & listTypesToDefine,
	_Inout_ std::ostringstream& oss)
{
	
	std::list<TypeToDefine> listAdditionalType;
	BOOL					bResult;

	listTypesToDefine.sort();
	listTypesToDefine.unique();


	for(auto iter=listTypesToDefine.begin(); iter!=listTypesToDefine.end(); iter++)
	{
		bResult = printType(
			pContext, 
			*iter, 
			listAdditionalType, 
			oss);

		if(bResult == FALSE)
		{
			oss << std::endl << "[ERROR] dumpTypeList : unable to  print type\n" << std::endl;
			return FALSE;
		}
	}
	
	return TRUE;
}


// ---------------------------------------------------------------------------------------------
BOOL __fastcall RpcDecompilerPrintAllTypesInList
	(
	_In_ VOID* pContext, 
	_In_ std::list<TypeToDefine> & listProcTypes,
	_Out_ std::ostringstream& oss
	)
{
	std::list<TypeToDefine> listAllTypesSorted;
	std::ostringstream ossUseless;
	

	/*oss<<"/*<DEBUG purpose>"<<std::endl;
	for(auto iter = listProcTypes.begin(); iter != listProcTypes.end(); iter++)
	{
		oss<<"offset : "<<iter->uOffset<<" ";
	}
	oss<<"</DEBUG purpose>"<<std::endl;*/
	




	// recursively explore all list in order to get a complete list of types
	if (getAllTypesSortedInAList(pContext, listProcTypes, listAllTypesSorted, ossUseless) == FALSE)
	{
		oss<<"[ERROR] unable to get list of all types sorted" << std::endl;
		return FALSE;
	}

	if( dumpTypeList(pContext, listAllTypesSorted, oss) == FALSE)
	{
		oss<<"[ERROR] unable to dump type list" << std::endl;
	}
	

	return TRUE;

	
}


//----------------------------------------------------------------------------
BOOL	__fastcall RpcDecompilerDecodeAndPrintPrototypeReturnType(
	_In_	VOID* pContext, 
	_In_	UINT formatStringOffset, 
	_Out_	UINT * paramOffset,
	_Out_	UINT * sizeOfProcDescr,
	_Inout_ IdlFunctionDesc& IdlFunctionDesc,
	_Inout_	std::list<TypeToDefine>& listProcType,
	_Inout_	std::ostringstream& oss)
{
	BOOL						bResult				= FALSE;
	RpcDecompilerCtxt_T *		pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;

	Oi_Header_1stPart_t			oiHeader1stPartToDecode;
	Oi_Header_RpcFlags_t *		oiHeaderRpcFlagsToDecode = NULL;
	Oi_Header_3rdPart_t			oiHeader3rdPartToDecode;
	ExplicitHandle_t *			oiHeaderExplicitHandleToDecode = NULL;
	Oif_Header_t				oifHeaderToDecode;
	Win2kExt_Header_t			w2kHeaderToDecode;

	UINT						currentOffset		= formatStringOffset;
	UINT						numParam			= 0;
	BOOL						isReturnParam		= FALSE;
	BOOL						returnTypeHasBeenPrinted = FALSE;

	UINT						paramSizeInBytes	= 0;

	memset(&oiHeader1stPartToDecode, 0, sizeof(Oi_Header_1stPart_t));
	memset(&oiHeader3rdPartToDecode, 0, sizeof(Oi_Header_3rdPart_t));
	memset(&oifHeaderToDecode, 0, sizeof(Oif_Header_t));
	memset(&w2kHeaderToDecode, 0, sizeof(Win2kExt_Header_t));
	
	//following checks already done by the caller function
	/*	if (pRpcDecompilerCtxt == NULL) goto End;
	if (pRpcDecompilerCtxt->pRpcViewHelper == NULL) goto End;

	if (pRpcDecompilerCtxt->pRpcDecompilerInfo == NULL) goto End;
	if (pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString == NULL) goto End;*/

	RPC_DEBUG_FN((UCHAR*)"\n\n*************** RpcDecompilerDecodeAndPrintPrototypeReturnType ******************");

	/**********************************************************************
	* Decode the Oi Header
	***********************************************************************/
	//Decode the Oi header 1st part
	RPC_GET_PROCESS_DATA(
		(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString + currentOffset),
		&oiHeader1stPartToDecode, 
		sizeof(Oi_Header_1stPart_t)
		);
	currentOffset += sizeof(Oi_Header_1stPart_t);

	RPC_DEBUG_FN((UCHAR*)"\n\n");
	RPC_DEBUG_FN((UCHAR*)"\noiHeader1stPartToDecode.handle_type = 0x%x", oiHeader1stPartToDecode.handle_type);
	RPC_DEBUG_FN((UCHAR*)"\noiHeader1stPartToDecode.oi_flags = 0x%x", oiHeader1stPartToDecode.oi_flags);

	//Decode the Oi header Rpc Flags
	if(oiHeader1stPartToDecode.oi_flags & Oi_HAS_RPCFLAGS)
	{	
		oiHeaderRpcFlagsToDecode = (Oi_Header_RpcFlags_t *) RPC_ALLOC_FN(sizeof(Oi_Header_RpcFlags_t));
		if(oiHeaderRpcFlagsToDecode == NULL)
		{
			RPC_DEBUG_FN((UCHAR*)"\n!!! source = %s line = %d - RpcAlloc returned NULL pointer", __FILE__, __LINE__);
			goto End;
		}

		RPC_GET_PROCESS_DATA(
			(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString + currentOffset),
			oiHeaderRpcFlagsToDecode, 
			sizeof(Oi_Header_RpcFlags_t)
			);	

		currentOffset += sizeof(Oi_Header_RpcFlags_t);
		//RPC_DEBUG_FN((UCHAR*)"\noiHeaderRpcFlagsToDecode->rpc_flags = 0x%x", oiHeaderRpcFlagsToDecode->rpc_flags);
	}



	//Decode the Oi header 3rd part
	RPC_GET_PROCESS_DATA(
		(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString + currentOffset),
		&oiHeader3rdPartToDecode, 
		sizeof(Oi_Header_3rdPart_t)
		);	
	currentOffset += sizeof(Oi_Header_3rdPart_t);

	RPC_DEBUG_FN((UCHAR*)"\noiHeader3rdPartToDecode.procNum = 0x%x", oiHeader3rdPartToDecode.procNum);
	RPC_DEBUG_FN((UCHAR*)"\noiHeader3rdPartToDecode.stack_size = 0x%x", oiHeader3rdPartToDecode.stack_size);


	//Decode the Oi header explicit handle
	//TODO : creer une fonction qui fait cela, cad GetExplicitHandleSize
	if(oiHeader1stPartToDecode.handle_type == FC_EXPLICIT_HANDLE)
	{
		oiHeaderExplicitHandleToDecode = (ExplicitHandle_t *) RPC_ALLOC_FN(sizeof(ExplicitHandle_t));
		if(oiHeaderExplicitHandleToDecode == NULL)
		{
			RPC_DEBUG_FN((UCHAR*)"!!! source = %s line = %d - RpcAlloc returned NULL pointer", __FILE__, __LINE__);
			goto End;
		}

		RPC_GET_PROCESS_DATA(
			(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString + currentOffset),
			oiHeaderExplicitHandleToDecode, 
			EXPLICIT_HANDLE_MIN_SIZE
			);	
		
		RPC_DEBUG_FN((UCHAR*)"\noiHeaderExplicitHandleToDecode->htype = 0x%x", oiHeaderExplicitHandleToDecode->htype);

		switch(oiHeaderExplicitHandleToDecode->htype)
		{
		case FC_BIND_PRIMITIVE:
#if EXPLICIT_HANDLE_MIN_SIZE != EXPLICIT_HANDLE_PRIMITIVE_SIZE
#error check the value of EXPLICIT_HANDLE_MIN_SIZE, EXPLICIT_HANDLE_PRIMITIVE_SIZE and the code
#endif
			currentOffset += EXPLICIT_HANDLE_PRIMITIVE_SIZE;

			RPC_DEBUG_FN((UCHAR*)"\noiHeaderExplicitHandleToDecode->hContent.explicitHandlePrimitive.flag = 0x%x", oiHeaderExplicitHandleToDecode->hContent.explicitHandlePrimitive.flag);
			RPC_DEBUG_FN((UCHAR*)"\noiHeaderExplicitHandleToDecode->hContent.explicitHandlePrimitive.offset = 0x%x", oiHeaderExplicitHandleToDecode->hContent.explicitHandlePrimitive.offset);

			break;

		case FC_BIND_GENERIC:
			RPC_GET_PROCESS_DATA(
				(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString + currentOffset),
				oiHeaderExplicitHandleToDecode, 
				EXPLICIT_HANDLE_GENERIC_SIZE
				);	
			currentOffset += EXPLICIT_HANDLE_GENERIC_SIZE;

			RPC_DEBUG_FN((UCHAR*)"\noiHeaderExplicitHandleToDecode->hContent.explicitHandleGeneric.flagAndSize = 0x%x", oiHeaderExplicitHandleToDecode->hContent.explicitHandleGeneric.flagAndSize);
			RPC_DEBUG_FN((UCHAR*)"\noiHeaderExplicitHandleToDecode->hContent.explicitHandleGeneric.offset = 0x%x", oiHeaderExplicitHandleToDecode->hContent.explicitHandleGeneric.offset);
			RPC_DEBUG_FN((UCHAR*)"\noiHeaderExplicitHandleToDecode->hContent.explicitHandleGeneric.binding_routine_pair_index = 0x%x", oiHeaderExplicitHandleToDecode->hContent.explicitHandleGeneric.binding_routine_pair_index);
			RPC_DEBUG_FN((UCHAR*)"\noiHeaderExplicitHandleToDecode->hContent.explicitHandleGeneric.pad = 0x%x", oiHeaderExplicitHandleToDecode->hContent.explicitHandleGeneric.pad);

			break;

		case FC_BIND_CONTEXT:
			RPC_GET_PROCESS_DATA(
				(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString + currentOffset),
				oiHeaderExplicitHandleToDecode, 
				EXPLICIT_HANDLE_CONTEXT_SIZE
				);	
			currentOffset += EXPLICIT_HANDLE_CONTEXT_SIZE;

			RPC_DEBUG_FN((UCHAR*)"\noiHeaderExplicitHandleToDecode->hContent.explicitHandleContext.flags = 0x%x", oiHeaderExplicitHandleToDecode->hContent.explicitHandleContext.flags);
			RPC_DEBUG_FN((UCHAR*)"\noiHeaderExplicitHandleToDecode->hContent.explicitHandleContext.offset = 0x%x", oiHeaderExplicitHandleToDecode->hContent.explicitHandleContext.offset);
			RPC_DEBUG_FN((UCHAR*)"\noiHeaderExplicitHandleToDecode->hContent.explicitHandleContext.context_rundown_routine_index = 0x%x", oiHeaderExplicitHandleToDecode->hContent.explicitHandleContext.context_rundown_routine_index);
			RPC_DEBUG_FN((UCHAR*)"\noiHeaderExplicitHandleToDecode->hContent.explicitHandleContext.param_num = 0x%x", oiHeaderExplicitHandleToDecode->hContent.explicitHandleContext.param_num);

			break;

		default:
			RPC_DEBUG_FN((UCHAR*)"\n!!! source = %s line = %d - ERREUR : oiHeaderExplicitHandleToDecode->htype undefined value. value = 0x%x", __FILE__, __LINE__, oiHeaderExplicitHandleToDecode->htype);
			bResult = FALSE;
			goto End;
			break;

		} // switch(oiHeaderExplicitHandleToDecode->htype)

	}//	if(oiHeader1stPartToDecode.handle_type == FC_EXPLICIT_HANDLE)


	/**********************************************************************
	* Decode the Oif Header (necessary for 64bits)
	***********************************************************************/

	RPC_GET_PROCESS_DATA(
		(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString + currentOffset),
		&oifHeaderToDecode, 
		OIF_EXT_HEADER_SIZE
		);	
	currentOffset += OIF_EXT_HEADER_SIZE;

	RPC_DEBUG_FN((UCHAR*)"\noifHeaderToDecode.constant_client_buffer_size = 0x%x", oifHeaderToDecode.constant_client_buffer_size);
	RPC_DEBUG_FN((UCHAR*)"\noifHeaderToDecode.constant_server_buffer_size = 0x%x", oifHeaderToDecode.constant_server_buffer_size);
	RPC_DEBUG_FN((UCHAR*)"\noifHeaderToDecode.interpreter_opt_flag = 0x%x", oifHeaderToDecode.interpreter_opt_flag);
	RPC_DEBUG_FN((UCHAR*)"\noifHeaderToDecode.number_of_param = 0x%x", oifHeaderToDecode.number_of_param);


#ifdef DBG_DECOMP
	oss << "/* is Async bit set : ";

	if(oifHeaderToDecode.interpreter_opt_flag.HasAsyncUuid)
	{
		oss << " TRUE */";
	}
	else
	{
		oss << " FALSE */";
	}
#endif

	/**********************************************************************
	* Decode the Win2kExt Header
	***********************************************************************/
	// TODO : need to identify which precise condition trigger the apparition of a win2KExt header
	if(pRpcDecompilerCtxt->pRpcDecompilerInfo->NDRVersion != 0x20000)
	{
		RPC_GET_PROCESS_DATA(
			(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString + currentOffset),
			&(w2kHeaderToDecode.extension_version), 
			sizeof(w2kHeaderToDecode.extension_version)
			);

		switch (w2kHeaderToDecode.extension_version)
		{
		case WIN2K_EXT_HEADER_32B_SIZE:
			RPC_GET_PROCESS_DATA(
				(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString + currentOffset),
				&w2kHeaderToDecode, 
				WIN2K_EXT_HEADER_32B_SIZE
				);

			currentOffset += WIN2K_EXT_HEADER_32B_SIZE;

			break;
		case WIN2K_EXT_HEADER_64B_SIZE:
			RPC_GET_PROCESS_DATA(
				(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString + currentOffset),
				&w2kHeaderToDecode, 
				WIN2K_EXT_HEADER_64B_SIZE
				);

			currentOffset += WIN2K_EXT_HEADER_64B_SIZE;

			break;
		default:
			RPC_DEBUG_FN((UCHAR*)"\n!!! source = %s line = %d - ERREUR : w2kHeaderToDecode.extension_version undefined value. value = 0x%x", __FILE__, __LINE__, w2kHeaderToDecode.extension_version);
			bResult = FALSE;
			goto End;
		}
					// parse Win32Ext Header
		IdlFunctionDesc.parseWin32ExtHeader(&w2kHeaderToDecode);

		RPC_DEBUG_FN((UCHAR*)"\nw2kHeaderToDecode.extension_version = 0x%x", w2kHeaderToDecode.extension_version);
		RPC_DEBUG_FN((UCHAR*)"\nw2kHeaderToDecode.interpreter_opt_flag2 = 0x%x", w2kHeaderToDecode.interpreter_opt_flag2);
		RPC_DEBUG_FN((UCHAR*)"\nw2kHeaderToDecode.clientCorrHint = 0x%x", w2kHeaderToDecode.clientCorrHint);
		RPC_DEBUG_FN((UCHAR*)"\nw2kHeaderToDecode.serverCorrHint = 0x%x", w2kHeaderToDecode.serverCorrHint);
		RPC_DEBUG_FN((UCHAR*)"\nw2kHeaderToDecode.notifyIndex = 0x%x", w2kHeaderToDecode.notifyIndex);

		if(w2kHeaderToDecode.extension_version == WIN2K_EXT_HEADER_64B_SIZE)
		{
			RPC_DEBUG_FN((UCHAR*)"\nw2kHeaderToDecode.floatDoubleMask = 0x%x", w2kHeaderToDecode.floatDoubleMask);
		}

	
	}//endif
	



	//TODO
	// Print the handle type information to take into account
	switch(oiHeader1stPartToDecode.handle_type)
	{
	case FC_BIND_CONTEXT:
		break;
	case FC_BIND_PRIMITIVE:
		
		oss << "\n/*********************************************************";
		oss << "\n * Add the following line in header of the the ACF file:";
		//oss << "\n *\timplicit_handle (handle_t "<<narrow(std::wstring(DEFAULT_IF_NAME))<<"_IfHandle)";
		oss << "\n *\timplicit_handle (handle_t "<< DEFAULT_IF_NAME <<"_IfHandle)";
		oss << "\n *********************************************************/\n";
		break;
	case FC_AUTO_HANDLE:
		break;
	case FC_CALLBACK_HANDLE:
		oss << "[callback]\t";
		break;
	case FC_EXPLICIT_HANDLE:
		break;
	default:
		RPC_DEBUG_FN((UCHAR*)"\n!!! source = %s line = %d - ERREUR : oiHeader1stPartToDecode.handle_type undefined value. value = 0x%x", __FILE__, __LINE__, oiHeader1stPartToDecode.handle_type);
		bResult = FALSE;
		goto End;
		break;
	}

	// Compute the nbParamToPrint and the paramOffset
	*paramOffset = currentOffset;
	IdlFunctionDesc.setNbParam(oifHeaderToDecode.number_of_param); //TODO : cas du oi header ?
	
	// Find and print the procedure return type
	if( oifHeaderToDecode.interpreter_opt_flag.HasReturn )
	{
		if (oifHeaderToDecode.number_of_param <= 0)
		{
			//No parameter to be printed whereas the procedure is supposed to return something...
			RPC_DEBUG_FN((UCHAR*)"\n!!! RpcDecompilerDecodeAndPrintPrototypeReturnType: file = %s, line = %d, *nbParamToPrint = %d whereas oifHeaderToDecode.interpreter_opt_flag.HasReturn",
				__FILE__, __LINE__, oifHeaderToDecode.number_of_param);
			goto End;
		}
		else //(oifHeaderToDecode.number_of_param > 0)
		{
			//Find and print the return parameter type. Note: as soon as a return type is printed, the loop ends
			numParam = 1;
			while( (! returnTypeHasBeenPrinted) && (numParam <= oifHeaderToDecode.number_of_param) )
			{
				RPC_DEBUG_FN((UCHAR*)"\nRpcDecompilerDecodeAndPrintPrototypeReturnType: numParam = 0x%x on total to analyse = 0x%x", numParam, oifHeaderToDecode.number_of_param);
				bResult = RpcDecompilerGetReturnParamInfo(/* in */ pRpcDecompilerCtxt, /* in */ currentOffset, /* in */ paramDescrOif, /* out */ &isReturnParam);
				if (bResult == FALSE) goto End;				

				if(isReturnParam)
				{
					//Return param is found, to be printed
					//RPC_PRINT_FN((UCHAR*)"\t");
					oss << "\t";

					bResult = RpcDecompilerPrintParam(
						pRpcDecompilerCtxt,
						/* in */	currentOffset, 
						/* in */	paramDescrOif, 
						/* out */	&paramSizeInBytes,
						/* in */	IdlFunctionDesc,
						/* in/out */ listProcType,
						/* in, out */oss);


					if (bResult == FALSE || paramSizeInBytes == RPC_DECOMPILER_INVALID_PARAM_SIZE) goto End;
					returnTypeHasBeenPrinted = TRUE;
				}

				currentOffset += OIF_PARAM_SIZE;
				numParam++;
			}

			if(! returnTypeHasBeenPrinted)
			{
				//No parameter to be printed whereas the procedure is supposed to return something...
				RPC_DEBUG_FN((UCHAR*)"\n!!! RpcDecompilerDecodeAndPrintPrototypeReturnType: file = %s, line = %d, no return param found whereas oifHeaderToDecode.interpreter_opt_flag.HasReturn",
					__FILE__, __LINE__);
				goto End;
			}
		}// (*nbParamToPrint > 0)
	}//( oifHeaderToDecode.interpreter_opt_flag.HasReturn )
	else
	{
		//RPC_PRINT_FN((UCHAR*)"\tvoid");
		oss <<"\tvoid ";

		RPC_DEBUG_FN((UCHAR*)"\tvoid ");
	}// !( oifHeaderToDecode.interpreter_opt_flag.HasReturn )

End:
	*sizeOfProcDescr = currentOffset - formatStringOffset;
	if(oiHeaderRpcFlagsToDecode != NULL) RPC_FREE_FN(oiHeaderRpcFlagsToDecode);
	if(oiHeaderExplicitHandleToDecode != NULL) RPC_FREE_FN(oiHeaderExplicitHandleToDecode);
	return (bResult);
}


//------------------------------------------------------------------------------
BOOL __fastcall RpcDecompilerPrintPrototypeName(
	_In_	VOID* pContext, 
	_In_	UINT ProcIndex,
	_Inout_	std::ostringstream& oss)
{
	RpcDecompilerCtxt_T *		pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;

	//following checks already done by the caller function
	/*	if (pRpcDecompilerCtxt == NULL) goto End;
	if (pRpcDecompilerCtxt->pRpcViewHelper == NULL) goto End;

	if (pRpcDecompilerCtxt->pRpcDecompilerInfo == NULL) goto End;*/
	if(pRpcDecompilerCtxt->pRpcDecompilerInfo->ppProcNameTable == NULL)
	{
		return FALSE;
	}

	RPC_DEBUG_FN((UCHAR*)"\n\n************************** RpcDecompilerPrintPrototypeName ************************");


	if(pRpcDecompilerCtxt->pRpcDecompilerInfo->ppProcNameTable[ProcIndex] == NULL)
	{
	
		//oss << "_Function_0x" << std::hex << (long)(pFunction - pModuleBase) << "(";
		oss << " _Function_" << std::dec << ProcIndex << "(";

	}
	else
	{
		size_t sz = wcslen(pRpcDecompilerCtxt->pRpcDecompilerInfo->ppProcNameTable[ProcIndex]) + 1;
		size_t szConverted = 0;

		char* pTmp = (char*)pRpcDecompilerCtxt->pRpcViewHelper->RpcAlloc(sz);

		if(pTmp != NULL)
		{
			ZeroMemory(pTmp, sz);
			wcstombs_s(&szConverted, pTmp, sz, pRpcDecompilerCtxt->pRpcDecompilerInfo->ppProcNameTable[ProcIndex], sz);
			
			oss << pTmp << "(";

			pRpcDecompilerCtxt->pRpcViewHelper->RpcFree(pTmp);
		}
		
		//oss << narrow(std::wstring(pRpcDecompilerCtxt->pRpcDecompilerInfo->ppProcNameTable[ProcIndex])) << "(";


	}//	if(pRpcDecompilerCtxt->pRpcDecompilerInfo->ppProcNameTable[ProcIndex] == NULL)


	return TRUE;
}


//------------------------------------------------------------------------------
BOOL __fastcall RpcDecompilerGetReturnParamInfo(
	_In_	VOID* pContext, 
	_In_	UINT paramOffset, 
	_In_	ParamID_E paramDescrFormat, 
	_Out_	BOOL * isReturnParam)
{
	BOOL						bResult				= FALSE;
	RpcDecompilerCtxt_T *		pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;

	//UINT						formatStringOffset	= 0;
	ProcFormatStringParam_U 	paramToAnalyze;
	

	//following checks already done by the caller function
	/*	if (pRpcDecompilerCtxt == NULL) goto End;
	if (pRpcDecompilerCtxt->pRpcViewHelper == NULL) goto End;

	if(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString == NULL) goto End; */

	RPC_DEBUG_FN((UCHAR*)"\n\n******************* RpcDecompilerGetReturnParamInfo *****************");

	switch(paramDescrFormat)
	{
	case paramDescrOi:
		//TODO
		//paramSizeInBytes = OI_PARAM_BASETYPE_SIZE;
		//paramSizeInBytes = OI_PARAM_OTHERTYPE_SIZE;
		RPC_DEBUG_FN((UCHAR*)"\n!!! source = %s ligne = %d - RpcDecompilerGetReturnParamInfo : paramDescrOi not handled", __FILE__, __LINE__);
		goto End;
		break;

	case paramDescrOif:
		RPC_GET_PROCESS_DATA(
			(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString + paramOffset),
			&paramToAnalyze, 
			sizeof(paramToAnalyze)
			);	

		RPC_DEBUG_FN((UCHAR*)"\nRpcDecompilerGetReturnParamInfo: paramToPrint.oif_Format.paramAttributes = 0x%x", paramToAnalyze.oif_Format.paramAttributes);

		*isReturnParam = paramToAnalyze.oif_Format.paramAttributes.IsReturn;
		break;
	default:
		RPC_DEBUG_FN((UCHAR*)"\n!!! RpcDecompilerGetReturnParamInfo : ERROR, paramDescrFormat not recognized");
		bResult = FALSE;
		goto End;
		break;
	}

End:
	return bResult;
}


// -----------------------------------------------------------------------------------------------
DWORD __fastcall getSimpleTypeMemorySize(_In_ FC_TYPE fcType)
{
		switch(fcType)
	{

	case FC_BYTE:
	case FC_CHAR:
	case FC_SMALL:
	case FC_USMALL:
		return 1;

	case FC_WCHAR:
	case FC_SHORT:
	case FC_USHORT:
		return 2;

	case FC_LONG:
	case FC_ULONG:
	case FC_ENUM16:
	case FC_ENUM32:
	case FC_ERROR_STATUS_T:
		return 4;

	case FC_FLOAT:
	case FC_HYPER:
	case FC_DOUBLE:
	case FC_INT3264:
	case FC_UINT3264:
	case FC_SYSTEM_HANDLE:
		return 8;

	case FC_ZERO:
		return 0;

	case FC_IGNORE:
		return POINTER_SIZE;

	default:
		return (DWORD)-1;

	}
}


// ------------------------------------------------------------------------------------------------
BOOL __fastcall processSimpleType(
	_In_	VOID* pContext,
	_In_	FC_TYPE  fcType,
	_Inout_	ParamDesc& paramDesc,
	_Inout_	std::ostringstream& oss)
{
	if (printSimpleType(pContext, fcType, oss) == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] processSimpleType : unknow type\n");
		return FALSE;
	}

	// get simple type size
	//paramDesc.setMemorySize(getSimpleTypeMemorySize(fcType));

	displayPtrLevel(paramDesc.getuPtrLevel(), oss);

	oss<<paramDesc.getStrTypeName();

	return TRUE;
}


//------------------------------------------------------------------------------
BOOL __fastcall printSimpleType(
	_In_	VOID* pContext, 
	_In_	FC_TYPE  fcType,
	_Inout_ std::ostringstream& oss)
{

    UNREFERENCED_PARAMETER(pContext);

	switch(fcType)
	{
	case FC_BYTE:
		oss<<"byte ";
		break;
	case FC_CHAR:
		oss<<"char ";
		break;
	
	case FC_SMALL:
		oss<<"small ";
		break;

	case FC_USMALL:
		oss<<"unsigned small ";
		break;
	
	case FC_WCHAR:
		oss <<"wchar_t ";
	break;

	case FC_SHORT:
		oss << "short ";
		break;
	case FC_USHORT:
		oss << "unsigned short ";
		break;

	case FC_LONG:
		oss << "long ";
		break;

	case FC_ULONG:
		oss << "unsigned long ";
		break;

	case FC_FLOAT:
		oss << "float ";
		break;

	case FC_HYPER:
		oss << "hyper ";
		break;
		
	case FC_DOUBLE:
		oss << "double ";
		break;

	case FC_ENUM16:
		oss << "/* enum_16 */ short ";
		break;

	case FC_ENUM32:
		oss << " /* enum_32 : typedef [v1_enum] enum  */ long ";
		break;

	case FC_ERROR_STATUS_T:
		oss << "error_status_t ";
		break;

	case FC_IGNORE:
		oss << "void*  /* FC_IGNORE */ ";
		break;

	case FC_INT3264:
		oss <<"__int3264 ";
		break;

	case FC_UINT3264:
		oss << "unsigned __int3264 ";
		break;

	case FC_SYSTEM_HANDLE:
		oss << "/* FC_SYSTEM_HANDLE */ hyper ";
		break;

	default :
		oss << "[ERROR] parseBaseType : unknown type ("<<fcType<<")";
		return FALSE;  // TODO : ret FALSE

	}

	return  TRUE;
}


//------------------------------------------------------------------------------
BOOL __fastcall rpcDumpType(
	_In_	VOID* pContext,
	_In_	ParamDesc& paramDesc,
	_Inout_	std::list<TypeToDefine>& listProcTypes,
	_Inout_	std::ostringstream& oss)
{
	BOOL						bResult				= TRUE;
	RpcDecompilerCtxt_T *		pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	BYTE bFC_TYPE;
	

	RPC_GET_PROCESS_DATA(
			paramDesc.getRva(),
			&bFC_TYPE, 
			sizeof(bFC_TYPE));	

	if(bResult == FALSE)
	{
		RPC_ERROR_FN("RpcGetProcessData failed\n");
		return FALSE;
	}

	paramDesc.setFcType((FC_TYPE)bFC_TYPE);

	switch(paramDesc.getFcType())
	{
			//------------------------------------
			//    Simple type
			//------------------------------------
		case FC_BYTE:
		case FC_CHAR:
		case FC_SMALL:
		case FC_WCHAR:
		case FC_SHORT:
		case FC_USHORT:
		case FC_LONG:
		case FC_ULONG:
		case FC_FLOAT:
		case FC_HYPER:
		case FC_DOUBLE:
		case FC_ENUM16:
		case FC_ENUM32:
		case FC_ERROR_STATUS_T:
		case FC_IGNORE:
		case FC_INT3264:
		case FC_UINT3264:
		case FC_SYSTEM_HANDLE:
			//processSimpleType(pContext, 
			bResult = processSimpleType(pContext, (FC_TYPE)bFC_TYPE, paramDesc,  oss);
			if (bResult==FALSE) RPC_ERROR_FN("processSimpleType failed\n");
			break;



			//------------------------------------
			//    Arrays
			//------------------------------------
		case FC_CARRAY:
			bResult = processConformantArray(pContext,paramDesc.getRva(), (FC_TYPE)bFC_TYPE, paramDesc, listProcTypes, oss);
			if (bResult == FALSE) RPC_ERROR_FN("processConformantArray failed\n");
			break;
		case FC_CVARRAY:
			bResult = processConformantVaryingArray(pContext,paramDesc.getRva(), (FC_TYPE)bFC_TYPE, paramDesc, listProcTypes, oss);
			if (bResult == FALSE) RPC_ERROR_FN("processConformantVaryingArray failed\n");
			break;
		case FC_SMFARRAY:
		case FC_LGFARRAY:
			bResult = processFixedSizeArray(pContext,paramDesc.getRva(), (FC_TYPE)bFC_TYPE, paramDesc, listProcTypes, oss);
			if (bResult == FALSE) RPC_ERROR_FN("processFixedSizeArray failed\n");
			break;
		case FC_SMVARRAY:
		case FC_LGVARRAY:
			bResult = processVaryingArray(pContext,paramDesc.getRva(), (FC_TYPE)bFC_TYPE, paramDesc, listProcTypes, oss);
			if (bResult == FALSE) RPC_ERROR_FN("processVaryingArray failed\n");
			break;
		case FC_BOGUS_ARRAY:
			bResult = processComplexArray(pContext,paramDesc.getRva(), (FC_TYPE)bFC_TYPE, paramDesc, listProcTypes, oss);
			if (bResult == FALSE) RPC_ERROR_FN("processComplexArray failed\n");
			break;
			//------------------------------------
			//    Structures
			//------------------------------------
		case FC_STRUCT:
		case FC_PSTRUCT: 
		case FC_CSTRUCT:
		case FC_CPSTRUCT:
		case FC_CVSTRUCT:
		case FC_BOGUS_STRUCT:
		case FC_HARD_STRUCT:
			bResult = processStructure(pContext, paramDesc.getRva(), (FC_TYPE)bFC_TYPE, paramDesc, listProcTypes, oss);
			if (bResult == FALSE) RPC_ERROR_FN("processStructure failed\n");
			break;
			//------------------------------------
			//  Unions
			//------------------------------------
		case FC_ENCAPSULATED_UNION:

			bResult = processEncapsulatedUnion(
				pContext,
				paramDesc.getRva(),
				(FC_TYPE)bFC_TYPE,
				paramDesc, 
				listProcTypes,
				oss);
			if (bResult == FALSE) RPC_ERROR_FN("processEncapsulatedUnion failed\n");
			break;

		case FC_NON_ENCAPSULATED_UNION:

			bResult = processNonEncapsulatedUnion(
				pContext,
				paramDesc.getRva(),
				(FC_TYPE)bFC_TYPE,
				paramDesc, 
				listProcTypes,
				oss);
			if (bResult == FALSE) RPC_ERROR_FN("processNonEncapsulatedUnion failed\n");
			break;

			


			//------------------------------------
			//    Pointers
			//------------------------------------
		case FC_RP:
		case FC_UP:
		case FC_FP:
		case FC_OP:
			bResult = processStandardPointer(
				pContext,
				paramDesc.getRva(),
				(FC_TYPE)bFC_TYPE,
				paramDesc, 
				listProcTypes,
				oss);
			if (bResult == FALSE) RPC_ERROR_FN("processStandardPointer failed\n");
			break;

		case FC_IP:
			bResult = processInterfacePointer(
				pContext,
				paramDesc.getRva(),
				paramDesc, 
				oss);
			if (bResult == FALSE) RPC_ERROR_FN("processInterfacePointer failed\n");
			break;

		case FC_BYTE_COUNT_POINTER:
			bResult = processByteCountPointer(
				pContext,
				paramDesc.getRva(),
				paramDesc, 
				listProcTypes,
				oss
				);
			if (bResult == FALSE) RPC_ERROR_FN("processByteCountPointer failed\n");
			break;

			//------------------------------------
			//    Strings
			//------------------------------------
		case FC_C_CSTRING:
		case FC_C_WSTRING:
			bResult = processConformantString(pContext, paramDesc.getRva(), (FC_TYPE)bFC_TYPE, paramDesc, oss);
			if (bResult == FALSE) RPC_ERROR_FN("processConformantString failed\n");
			break;

		case FC_WSTRING:
		case FC_CSTRING:
			bResult = processNonConformantString(pContext, paramDesc.getRva(), (FC_TYPE)bFC_TYPE, paramDesc, oss);
			if (bResult == FALSE) RPC_ERROR_FN("processNonConformantString failed\n");
			break;

		case FC_SSTRING:
		case FC_C_SSTRING:
			bResult = processStructureString(pContext, paramDesc.getRva(), (FC_TYPE)bFC_TYPE, paramDesc, oss);
			if (bResult == FALSE) RPC_ERROR_FN("processStructureString failed\n");
			break;

			//------------------------------------
			//    Misc ...
			//------------------------------------
		case FC_BIND_CONTEXT:
			bResult = processBindContext(pContext,paramDesc.getRva(), paramDesc, listProcTypes, oss);
			if (bResult == FALSE) RPC_ERROR_FN("processBindContext failed\n");
			break;

		case FC_BLKHOLE:
			bResult = process_FC_BLKHOLE(pContext, paramDesc.getRva(), paramDesc, listProcTypes, oss);
			if (bResult == FALSE) RPC_ERROR_FN("process_FC_BLKHOLE failed\n");
			break;

		case FC_RANGE:
			bResult = processRange(pContext, paramDesc.getRva(), paramDesc, listProcTypes, oss);
			if (bResult == FALSE) RPC_ERROR_FN("processRange failed\n");
			break;

		case FC_USER_MARSHAL:
			bResult = processUserMarshal(pContext, paramDesc.getRva(), paramDesc, listProcTypes, oss);
			if (bResult == FALSE) RPC_ERROR_FN("processUserMarshal failed\n");
			break;

		case FC_TRANSMIT_AS:
		case FC_REPRESENT_AS:
			bResult = processTransmitRepresentAs(pContext, paramDesc.getRva(), paramDesc, listProcTypes, oss);
			if (bResult == FALSE) RPC_ERROR_FN("processTransmitRepresentAs failed\n");
			break;

		case FC_PIPE:
			bResult = processPipe(pContext, paramDesc.getRva(), paramDesc, listProcTypes, oss);
			if (bResult == FALSE) RPC_ERROR_FN("processPipe failed\n");
			break;

		case FC_ZERO:

			oss << "/* FC_ZERO */";
			bResult = TRUE;
			break;
		default:
			RPC_ERROR_FN("Invalid type\n");
			oss << "[ERROR] dump type : unknown_type (0x" << std::hex << (int)bFC_TYPE << ") ";
			oss<<paramDesc.getStrTypeName();
			bResult = TRUE;
			return bResult;
			
		return FALSE;
	}

	
	return bResult;
}


//-----------------------------------------------------------------------------
BOOL __fastcall getTypeMemorySize(
	_In_	VOID*	pContext,
	_In_	RVA_T	pType,
	_Out_	size_t*	pszMemorySize,
	_In_	BOOL	bHasRangeOnConformance)
{
	BOOL						bResult				= FALSE;
	RpcDecompilerCtxt_T *		pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;
	BYTE bFC_TYPE;
	
	*pszMemorySize = 0;

	RPC_GET_PROCESS_DATA(
			pType,
			&bFC_TYPE, 
			sizeof(bFC_TYPE));	

	if(bResult == FALSE)
	{
		RPC_DEBUG_FN((UCHAR*)"[ERROR] getTypeMemorySize : Unable to get process data\n");
		return FALSE;
	}

	
	switch(bFC_TYPE)
	{
			//------------------------------------
			//    Simple type
			//------------------------------------
		case FC_BYTE:
		case FC_CHAR:
		case FC_SMALL:
		case FC_WCHAR:
		case FC_SHORT:
		case FC_USHORT:
		case FC_LONG:
		case FC_ULONG:
		case FC_FLOAT:
		case FC_HYPER:
		case FC_DOUBLE:
		case FC_ENUM16:
		case FC_ENUM32:
		case FC_ERROR_STATUS_T:
		case FC_IGNORE:
		case FC_INT3264:
		case FC_UINT3264:
		case FC_SYSTEM_HANDLE:
			//processSimpleType(pContext, 
			(*pszMemorySize) = getSimpleTypeMemorySize((FC_TYPE)bFC_TYPE);
			bResult = TRUE;
			break;



			//------------------------------------
			//    Arrays
			//------------------------------------
		case FC_CARRAY:
		case FC_CVARRAY:
		case FC_SMFARRAY:
		case FC_LGFARRAY:
		case FC_SMVARRAY:
		case FC_LGVARRAY:
		case FC_BOGUS_ARRAY:
			*pszMemorySize = getArrayMemorySize(pContext, pType);
			break;


			//------------------------------------
			//    Structures
			//------------------------------------
		case FC_STRUCT:
		case FC_PSTRUCT: 
		case FC_CSTRUCT:
		case FC_CPSTRUCT:
		case FC_CVSTRUCT:
		case FC_BOGUS_STRUCT:
		case FC_HARD_STRUCT:
			bResult = getStructureMemorySize(pContext, pType, pszMemorySize);
			
			break;

			//------------------------------------
			//  Unions
			//------------------------------------
		case FC_ENCAPSULATED_UNION:
			bResult = getEncapsulatedUnionMemorySize(pContext, pType, pszMemorySize);		
			break;

		case FC_NON_ENCAPSULATED_UNION:
			bResult = getNonEncapsulatedUnionMemorySize(pContext, pType, pszMemorySize, bHasRangeOnConformance);
			break;

			


			//------------------------------------
			//    Pointers / Strings / BIND_CONTEXT
			//------------------------------------
		case FC_RP:
		case FC_UP:
		case FC_FP:
		case FC_OP:
		case FC_IP:
		case FC_BYTE_COUNT_POINTER:
		case FC_C_CSTRING:
		case FC_C_WSTRING:
		case FC_WSTRING:
		case FC_CSTRING:
		case FC_BIND_CONTEXT:
			*pszMemorySize = POINTER_SIZE;
			bResult = TRUE;
			break;



			//------------------------------------
			//    Misc ...
			//------------------------------------
		case FC_BLKHOLE:
			bResult = getFC_BLKHOLEMemorySize(pContext, pType, pszMemorySize, bHasRangeOnConformance);
			break;

		case FC_RANGE:
			bResult = getRangeMemorySize(pContext, pType, pszMemorySize);
			break;

		case FC_USER_MARSHAL:
			bResult = getUserMarshallMemorySize(pContext, pType, pszMemorySize);
			break;

		case FC_TRANSMIT_AS:
		case FC_REPRESENT_AS:
			bResult = getTransmitAsRepresentAsMemorySize(pContext, pType, pszMemorySize);
			break;

		case FC_PIPE:
			bResult = getPipeMemorySize(pContext, pType, pszMemorySize, bHasRangeOnConformance);
			break;

		case FC_ZERO:
			*pszMemorySize = 0;
			bResult = TRUE;
			break;

		default:
			bResult = FALSE;
			RPC_DEBUG_FN("[ERROR] getTypeMemorySize : unable to get memory size for type : 0x%x \n", bFC_TYPE);
			
		return FALSE;
	}

	//// <DBG>
	//if(bResult == TRUE)
	//{
	//	RPC_PRINT_FN((UCHAR*)"getTypeMemorySize type :  %x, size : %x \n", bFC_TYPE, *pszMemorySize);
	//	
	//}
	//else
	//{
	//	RPC_PRINT_FN((UCHAR*)"getTypeMemorySize failed \n");
	//}
	//// </DBG>

	return bResult;
}


// ------------------------------------------------------------------------------------------------
BOOL __fastcall RpcDecompilerPrintParam(
	_In_	VOID* pContext, 
	_In_	UINT paramOffset, 
	_In_	ParamID_E paramDescrFormat, 
	_Out_	UINT * paramSizeInBytes,
	_In_	const IdlFunctionDesc& IdlFunctionDesc,
	_Inout_	std::list<TypeToDefine>& listProcType,
	_Inout_	std::ostringstream& oss)
{
	BOOL						bResult				= FALSE;
	RpcDecompilerCtxt_T *		pRpcDecompilerCtxt	= (RpcDecompilerCtxt_T *) pContext;

	//UINT						formatStringOffset	= 0;
	ProcFormatStringParam_U 	paramToPrint;

	UINT32						argNbr = 0;

	ParamDesc				ParamDesc;

	//following checks already done by the caller function
	/*	if (pRpcDecompilerCtxt == NULL) goto End;
	if (pRpcDecompilerCtxt->pRpcViewHelper == NULL) goto End;

	if(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString == NULL) goto End; */


	switch(paramDescrFormat)
	{
	case paramDescrOi:
		//TODO
		//paramSizeInBytes = OI_PARAM_BASETYPE_SIZE;
		//paramSizeInBytes = OI_PARAM_OTHERTYPE_SIZE;
		goto End;
		break;


	case paramDescrOif:
		*paramSizeInBytes = OIF_PARAM_SIZE;

		RPC_GET_PROCESS_DATA(
			(pRpcDecompilerCtxt->pRpcDecompilerInfo->pProcFormatString + paramOffset),
			&paramToPrint, 
			sizeof(paramToPrint)
			);	

		RPC_DEBUG_FN((UCHAR*)"\nparamToPrint.oif_Format.paramAttributes = 0x%x", paramToPrint.oif_Format.paramAttributes);
		RPC_DEBUG_FN((UCHAR*)"\nparamToPrint.oif_Format.stack_offset = 0x%x", paramToPrint.oif_Format.stack_offset);

		//*isReturnParam = paramToPrint.oif_Format.paramAttributes.IsReturn;


		if(is64B)	argNbr = paramToPrint.oif_Format.stack_offset / VIRTUAL_STACK_OFFSET_GRANULARITY_64B;
		else		argNbr = paramToPrint.oif_Format.stack_offset / VIRTUAL_STACK_OFFSET_GRANULARITY_32B;


		// add param IdlFunctionDesc properties
		if(IdlFunctionDesc.hasRangeOnConformance())
		{
			ParamDesc.setHasRangeOnConformance();
		}

		// fill param desc with ParamAttr
		ParamDesc.fillWithParamAttr(paramToPrint.oif_Format.paramAttributes);

		if( (!paramToPrint.oif_Format.paramAttributes.IsReturn) /*&&
																(typeFieldType != ptrCommonSimpleDescr) &&
																(typeFieldType != ptrCommonComplexDescr) &&
																(typeFieldType != ptrInterfaceConstantIID) &&
																(typeFieldType != ptrInterfaceNonConstantIIDDescr) &&
																(typeFieldType != ptrByteCountSimpleDescr) &&
																(typeFieldType != ptrByteCountComplexDescr)*/
																//		) { RPC_PRINT_FN((UCHAR*)"_%d", paramOffset); RPC_DEBUG_FN((UCHAR*)"_%d", paramOffset);}
																	) 
		{ 
			std::ostringstream ossTmp;
			ossTmp << "arg_" << argNbr;

			ParamDesc.setParamName(ossTmp.str());

			RPC_DEBUG_FN((UCHAR*)" arg_%d", argNbr);
			
			oss << std::endl << "\t\t";
			
			if(paramToPrint.oif_Format.paramAttributes.IsIn && paramToPrint.oif_Format.paramAttributes.IsOut) 
			{
				oss << "[in, out]"; 
				RPC_DEBUG_FN((UCHAR*)"[in, out]");
			}
			else if (paramToPrint.oif_Format.paramAttributes.IsIn) 
			{
				oss<<"[in]"; 
				RPC_DEBUG_FN((UCHAR*)"[in]");
			}
			else // paramAttributes.IsOut  
			{
				oss<<"[out]"; 
				RPC_DEBUG_FN((UCHAR*)"[out]");
			}
		}

		if(paramToPrint.oif_Format.paramAttributes.IsBasetype)
		{
			bResult = processSimpleType(
				pContext,  
				(FC_TYPE)paramToPrint.oif_Format.paramType.base_type_format_char.type_format_char, 
				ParamDesc, 
				oss);

			if(FALSE == bResult)
			{
				displayErrorMessage(oss, " unable to processSimpleType ");
				goto End;
			}


		} //if(paramToPrint.oif_Format.paramAttributes.IsBasetype)
		else
		{
			//ParamDesc.setuOffset(paramToPrint.oif_Format.paramType.other_type_offset);	// param offset in type format string
			ParamDesc.setRva(pRpcDecompilerCtxt->pRpcDecompilerInfo->pTypeFormatString + paramToPrint.oif_Format.paramType.other_type_offset);

			bResult = rpcDumpType(
				pContext,
				ParamDesc,
				listProcType,
				oss);

			if(bResult == FALSE)
			{
				displayErrorMessage(oss, " unable to dump type");
				//RPC_DEBUG_FN((UCHAR*)"\n!!! source = %s ligne = %d - PrintParamOfFieldType returned bResult = %d", __FILE__, __LINE__, bResult);
				goto End;
			}		
		}// !(paramToPrint.oif_Format.paramAttributes.IsBasetype)

		

		bResult = TRUE;
		break;


	default:
		RPC_DEBUG_FN((UCHAR*)"\n!!! RpcDecompilerPrintParam : ERROR, paramDescrFormat not recognized");
		bResult = FALSE;
		goto End;
		break;
	}

End:
	return (bResult);
}

